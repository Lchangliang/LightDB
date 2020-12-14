//
// Created by lightman on 2020-12-07.
//

#ifndef LIGHTDB_MEMTABLE_H
#define LIGHTDB_MEMTABLE_H
#include <string>

#include "LightDB/db.h"
#include "db/dbformat.h"
#include "db/skiplist.h"
#include "util/arena.h"
#include "LightDB/status.h"
namespace LightDB {

class InternalKeyComparator;
class MemTableIterator;

class MemTable {
public:
  explicit MemTable(const InternalKeyComparator& comparator);

  MemTable(const MemTable&) = delete;
  MemTable& operator=(const MemTable&) = delete;

  void Ref() { ++refs_; }

  void Unref() {
    --refs_;
    assert(refs_ >= 0);
    if (refs_ <= 0) {
      delete this;
    }
  }

  void Add(SequenceNumber seq, ValueType type, const Slice& key, const Slice& value);

  bool Get(const LookupKey& key, std::string* value, Status* s);

private:
  friend class MemTableIterator;
  //   friend class MemTableBackwardIterator;

  struct KeyComparator {
    const InternalKeyComparator comparator;
    explicit KeyComparator(const InternalKeyComparator& c) : comparator(c) {}
    int operator()(const char* a, const char* b) const;
  };

  typedef SkipList<const char*, KeyComparator> Table;

  ~MemTable();

  KeyComparator comparator_;
  int refs_;
  Arena arena_;
  Table table_;
};

}  // namespace LightDB
#endif  // LIGHTDB_MEMTABLE_H
