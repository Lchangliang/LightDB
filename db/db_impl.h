#ifndef DB_IMPL_H
#define DB_IMPL_H

#include <string>
#include <deque>
#include "LightDB/status.h"
#include "LightDB/db.h"
#include "LightDB/write_batch.h"
#include "port/port.h"
#include "db/dbformat.h"
#include "LightDB/comparator.h"
#include "db/memtable.h"
namespace LightDB {

class MemTable;

class DBImpl : public DB {
public:
    DBImpl(const std::string& dbname);
    DBImpl(const DBImpl&) = delete;
    DBImpl& operator=(const DBImpl&) = delete;

    ~DBImpl() override;

    virtual Status Put(const Slice& key, const Slice& value) override;
    virtual Status Delete(const Slice& key) override;
    virtual Status Write(WriteBatch* ipdates) override;
    virtual Status Get(const Slice& key, std::string* value) override;
private:
    friend class DB;
    struct Writer;

    std::deque<Writer*> writers_;
    const std::string dbname;
    const InternalKeyComparator internal_comparator_;
    MemTable* mem_;
    WriteBatch* tmp_batch_ ;
    uint64_t last_sequence_;
    port::Mutex mutex_;

    WriteBatch* BuildBatchGroup(Writer** last_writer);
};

}  // namespace LightDB
#endif