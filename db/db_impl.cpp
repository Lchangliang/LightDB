#include "db/db_impl.h"

#include "db/write_batch_internal.h"
#include "util/mutexlock.h"
namespace LightDB {

struct DBImpl::Writer {
  explicit Writer(port::Mutex* mu) : batch(nullptr), done(false), cv(mu) {}
  Status status;
  WriteBatch* batch;
  bool done;
  port::CondVar cv;
};

DBImpl::DBImpl(const std::string& dbname)
    : dbname(dbname),
      internal_comparator_(BytewiseComparator()),
      mem_(new MemTable(internal_comparator_)),
      tmp_batch_(new WriteBatch),
      last_sequence_(0) {
  mem_->Ref();
}

Status DBImpl::Put(const Slice& key, const Slice& val) { return DB::Put(key, val); }

Status DB::Put(const Slice& key, const Slice& value) {
  WriteBatch batch;
  batch.Put(key, value);
  return Write(&batch);
}

Status DBImpl::Delete(const Slice& key) { return DB::Delete(key); }

Status DB::Delete(const Slice& key) {
  WriteBatch batch;
  batch.Delete(key);
  return Write(&batch);
}

Status DBImpl::Get(const Slice& key, std::string* value) {
  Status s;
  MutexLock l(&mutex_);
  SequenceNumber snapshot;
  snapshot = last_sequence_;
  MemTable* mem = mem_;
  mem->Ref();
  {
    mutex_.Unlock();
    LookupKey lkey(key, snapshot);
    mem->Get(lkey, value, &s);
    mutex_.Lock();
  }
  mem->Unref();
  return s;
}

Status DBImpl::Write(WriteBatch* updates) {
  Writer w(&mutex_);
  w.batch = updates;
  w.done = false;

  MutexLock l(&mutex_);
  writers_.push_back(&w);
  while (!w.done && &w != writers_.front()) {
    w.cv.Wait();
  }
  if (w.done) {
    return w.status;
  }
  Status status;
  uint64_t last_sequence = last_sequence_;
  Writer* last_writer = &w;
  if (updates != nullptr) {
    WriteBatch* write_batch = BuildBatchGroup(&last_writer);
    WriteBatchInternal::SetSequence(write_batch, last_sequence_ + 1);
    last_sequence += WriteBatchInternal::Count(write_batch);
    {
      mutex_.Unlock();
      status = WriteBatchInternal::InsertInto(write_batch, mem_);
      mutex_.Lock();
    }
    if (write_batch == tmp_batch_) tmp_batch_->Clear();
    last_sequence_ = last_sequence;
  }

  while (true) {
    Writer* ready = writers_.front();
    writers_.pop_front();
    if (ready != &w) {
      ready->status = status;
      ready->done = true;
      ready->cv.Signal();
    }
    if (ready == last_writer) break;
  }

  if (!writers_.empty()) {
    writers_.front()->cv.Signal();
  }
  return status;
}

WriteBatch* DBImpl::BuildBatchGroup(Writer** last_writer) {
  assert(!writers_.empty());
  Writer* first = writers_.front();
  WriteBatch* result = first->batch;
  assert(result != nullptr);

  size_t size = WriteBatchInternal::ByteSize(first->batch);

  // Allow the group to grow up to a maximum size, but if the
  // original write is small, limit the growth so we do not slow
  // down the small write too much.
  size_t max_size = 1 << 20;
  if (size <= (128 << 10)) {
    max_size = size + (128 << 10);
  }

  *last_writer = first;
  std::deque<Writer*>::iterator iter = writers_.begin();
  ++iter;  // Advance past "first"
  for (; iter != writers_.end(); ++iter) {
    Writer* w = *iter;

    if (w->batch != nullptr) {
      size += WriteBatchInternal::ByteSize(w->batch);
      if (size > max_size) {
        // Do not make batch too big
        break;
      }

      // Append to *result
      if (result == first->batch) {
        // Switch to temporary batch instead of disturbing caller's batch
        result = tmp_batch_;
        assert(WriteBatchInternal::Count(result) == 0);
        WriteBatchInternal::Append(result, first->batch);
      }
      WriteBatchInternal::Append(result, w->batch);
    }
    *last_writer = w;
  }
  return result;
}

DB::~DB() = default;

Status DB::Open(const std::string& dbname, DB** dbptr) {
  *dbptr = nullptr;

  DBImpl* impl = new DBImpl(dbname);
  *dbptr = impl;

  return Status();
}

DBImpl::~DBImpl() {
  if (mem_ != nullptr) mem_->Unref();
  delete tmp_batch_;
}

}  // namespace LightDB