// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "LightDB/comparator.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <type_traits>

#include "LightDB/slice.h"
#include "util/no_destructor.h"

namespace LightDB {

Comparator::~Comparator() = default;

namespace {
class BytewiseComparatorImpl : public Comparator {
 public:
  BytewiseComparatorImpl() = default;
  int Compare(const Slice& a, const Slice& b) const override {
    return a.compare(b);
  }
};
}  // namespace

const Comparator* BytewiseComparator() {
  static NoDestructor<BytewiseComparatorImpl> singleton;
  return singleton.get();
}

}  // namespace leveldb
