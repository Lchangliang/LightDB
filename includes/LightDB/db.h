//
// Created by lightman on 2020-12-07.
//

#ifndef LIGHTDB_DB_H
#define LIGHTDB_DB_H

#include <string>

#include "LightDB/slice.h"
#include "LightDB/status.h"

namespace LightDB {

class WriteBatch;

class DB {
public:
  DB() = default;
  DB(const DB&) = delete;
  DB& operator=(const DB&) = delete;
  virtual ~DB();

  static Status Open(const std::string& dbname, DB** dbptr);
  virtual Status Put(const Slice& key, const Slice& value) = 0;
  virtual Status Delete(const Slice& key) = 0;
  virtual Status Write(WriteBatch* updates) = 0;
  virtual Status Get(const Slice& key, std::string* value) = 0;
};
}  // namespace LightDB
#endif  // CMAKE_LEARN_DB_H
