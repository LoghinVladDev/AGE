//
// Created by Vlad-Andrei Loghin on 07.07.23.
//

#include <core/lang/coro/Generator.hpp>
#include <gtest/gtest.h>

namespace {
using age::Generator;
template <typename T> struct Tracked {
  Tracked(T v) : value(v) { ++constructed; }
  Tracked() { ++defaulted; }
  Tracked(Tracked const& t) : value(t.value) { ++copied; }
  Tracked(Tracked&& t) : value(t.value) { ++moved; }
  auto operator=(Tracked const& t) -> Tracked& {
    if (this == &t) {
      return *this;
    }
    value = t.value;
    ++copyAssigned;
    return *this;
  }
  auto operator=(Tracked&& t) -> Tracked& {
    if (this == &t) {
      return *this;
    }
    value = t.value;
    ++moveAssigned;
    return *this;
  }
  T value;
  static inline int constructed = 0;
  static inline int defaulted = 0;
  static inline int copied = 0;
  static inline int moved = 0;
  static inline int copyAssigned = 0;
  static inline int moveAssigned = 0;
};
} // namespace

TEST(GeneratorTest, nothrowGenerator) {
  auto iota = [](int limit) -> Generator<int> {
    for (int i = 0; i < limit; ++i) {
      co_yield i;
    }
  };

  auto iotaTill10 = iota(10);
  int equiv = 0;
  while (!iotaTill10.empty()) {
    auto v = iotaTill10.get();
    ASSERT_EQ(v, equiv++);
  }
  ASSERT_EQ(equiv, 10);

  equiv = 0;
  for (auto e : iota(10)) {
    ASSERT_EQ(e, equiv++);
  }
  ASSERT_EQ(equiv, 10);
}

TEST(GeneratorTest, copyCount) {
  auto iotaTracked = [](int limit) -> Generator<Tracked<int>> {
    for (int i = 0; i < limit; ++i) {
      co_yield i;
    }
  };

  for (auto&& e : iotaTracked(10)) {
    (void) e;
  }

  ASSERT_EQ(Tracked<int>::constructed, 11);
  ASSERT_EQ(Tracked<int>::defaulted, 0);
  ASSERT_EQ(Tracked<int>::copied, 0);
  ASSERT_EQ(Tracked<int>::moved, 0);
  ASSERT_EQ(Tracked<int>::copyAssigned, 0);
  ASSERT_EQ(Tracked<int>::moveAssigned, 10);
}
