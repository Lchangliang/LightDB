//
// Created by lightman on 2020-12-07.
//

#ifndef LIGHTDB_DBFORMAT_H
#define LIGHTDB_DBFORMAT_H
#include <cassert>
#include <string>

#include "LightDB/comparator.h"
#include "LightDB/slice.h"
#include "util/coding.h"
namespace LightDB {

namespace config {

// 目前只实现level-0
static const int kNumLevels = 1;
// Level-0 compaction is started when we hit this many files.
static const int kL0_CompactionTrigger = 4;

// Soft limit on number of level-0 files.  We slow down writes at this point.
static const int kL0_SlowdownWritesTrigger = 8;

// Maximum number of level-0 files.  We stop writes at this point.
static const int kL0_StopWritesTrigger = 12;
}  // namespace config

class InternalKey;

enum ValueType { kTypeDeletion = 0x0, kTypeValue = 0x1 };

static const ValueType kValueTypeForSeek = kTypeValue;

typedef uint64_t SequenceNumber;

static const SequenceNumber kMaxSequenceNumber = ((0x1ull << 56) - 1);

struct ParsedInternalKey {
  Slice user_key;
  SequenceNumber sequence;
  ValueType type;

  ParsedInternalKey() {}
  ParsedInternalKey(const Slice& u, const SequenceNumber& seq, ValueType t) : user_key(u), sequence(seq), type(t) {}
};

inline size_t InternalKeyEncodingLength(const ParsedInternalKey& key) { return key.user_key.size() + 8; }

void AppendInternalKey(std::string* result, const ParsedInternalKey& key);

bool ParseInternalKey(const Slice& internal_key, ParsedInternalKey* result);

inline Slice ExtractUserKey(const Slice& internal_key) {
  assert(internal_key.size() >= 8);
  return Slice(internal_key.data(), internal_key.size() - 8);
}

class InternalKeyComparator : public Comparator {
private:
  const Comparator* user_comparator_;

public:
  explicit InternalKeyComparator(const Comparator* c) : user_comparator_(c) {}
  int Compare(const Slice& a, const Slice& b) const override;
  int Compare(const InternalKey& a, const InternalKey& b) const;
  const Comparator* user_comparator() const { return user_comparator_; }
};

class InternalKey {
private:
  std::string rep_;

public:
  InternalKey() {}
  InternalKey(const Slice& user_key, SequenceNumber s, ValueType t) {
    AppendInternalKey(&rep_, ParsedInternalKey(user_key, s, t));
  }
  bool DecodeFrom(const Slice& s) {
    rep_.assign(s.data(), s.size());
    return !rep_.empty();
  }
  Slice Encode() const {
    assert(!rep_.empty());
    return rep_;
  }
  Slice user_key() const { return ExtractUserKey(rep_); }
  void SetFrom(const ParsedInternalKey& p) {
    rep_.clear();
    AppendInternalKey(&rep_, p);
  }
  void Clear() { rep_.clear(); }
};

inline int InternalKeyComparator::Compare(const InternalKey& a, const InternalKey& b) const {
  return Compare(a.Encode(), b.Encode());
}

inline bool ParseInternalKey(const Slice& internal_key, ParsedInternalKey* result) {
  const size_t n = internal_key.size();
  if (n < 8) return false;
  uint64_t num = DecodeFixed64(internal_key.data() + n - 8);
  uint8_t c = num & 0xff;
  result->sequence = num >> 8;
  result->type = static_cast<ValueType>(c);
  result->user_key = Slice(internal_key.data(), n - 8);
  return (c <= static_cast<uint8_t>(kTypeValue));
}

class LookupKey {
private:
  const char* start_;
  const char* kstart_;
  const char* end_;
  char space_[200];  // Avoid allocation for short keys
public:
  LookupKey(const Slice& user_key, SequenceNumber sequence);
  LookupKey(const LookupKey&) = delete;
  LookupKey& operator=(const LookupKey&) = delete;

  ~LookupKey();

  Slice memtable_key() const { return Slice(start_, end_ - start_); }
  Slice internal_key() const { return Slice(kstart_, end_ - kstart_); }
  Slice user_key() const { return Slice(kstart_, end_ - kstart_ - 8); }
};

inline LookupKey::~LookupKey() {
  if (start_ != space_) delete[] start_;
}

}  // namespace LightDB
#endif  // LIGHTDB_DBFORMAT_H
