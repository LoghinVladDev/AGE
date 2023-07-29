#include <gtest/gtest.h>
#include <lang/array/ArrayRef.hpp>

namespace {
using age::ArrayRef;
using age::ref;
} // namespace

TEST(ArrayRefTest, construction) {
  ArrayRef<int> defConstructed;

  auto baseCdsArray = cds::arrayOf(1, 5, 4, 2, 3);
  auto fromCdsArray = ref(baseCdsArray);

  // TODO : cds::staticArrayOf
  auto baseCdsSArray = cds::StaticArray<int, 5> {1, 5, 4, 2, 3};
  auto fromCdsSArray = ref(baseCdsSArray);

  auto baseStdVector = std::vector<int> {1, 5, 4, 2, 3};
  auto fromStdVector = ref(baseStdVector);

  auto baseStdArray = std::array<int, 5> {1, 5, 4, 2, 3};
  auto fromStdArray = age::ref(baseStdArray); // lookup calls std::ref ???

  int baseCArray[5] = {1, 5, 4, 2, 3};
  auto fromCArray = ArrayRef(baseCArray, 5);

  ArrayRef fromInitializerList = {1, 5, 4, 2, 3};

  std::initializer_list<int> cList = {1, 5, 4, 2, 3};
  ASSERT_EQ(fromCdsArray, cList);
  ASSERT_EQ(fromCdsSArray, cList);
  ASSERT_EQ(fromStdVector, cList);
  ASSERT_EQ(fromStdArray, cList);
  ASSERT_EQ(fromCArray, cList);
}
