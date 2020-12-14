//
// Created by lightman on 2020-12-07.
//

#ifndef LIGHTDB_COMPARATOR_H
#define LIGHTDB_COMPARATOR_H
#include <string>

namespace LightDB {
class Slice;

class Comparator {
public:
  virtual ~Comparator();
  virtual int Compare(const Slice& a, const Slice& b) const = 0;
};

const Comparator* BytewiseComparator();
}  // namespace LightDB
#endif  // LIGHTDB_COMPARATOR_H
