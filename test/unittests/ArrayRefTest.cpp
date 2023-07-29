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
  auto fromStdArray = ref(baseStdArray); // lookup calls std::ref ???

  int baseCArray[] = {1, 5, 4, 2, 3};
  auto fromCArray = ArrayRef(baseCArray, sizeof(baseCArray) / sizeof(baseCArray[0]));
  auto fromCArrayUnsized = ArrayRef(baseCArray);

  // Temporary Extension required to avoid creation of a function to test this. Init list
  // Would destroy at end of stmt, but is extended.
  // ArrayRef fromInitializerList = {1, 5, 4, 2, 3};
  std::initializer_list<int> const& initListExtendedTemporary = {1, 5, 4, 2, 3};
  ArrayRef fromInitializerList = initListExtendedTemporary;

  std::initializer_list<int> cList = {1, 5, 4, 2, 3};
  ASSERT_EQ(fromCdsArray, cList);
  ASSERT_EQ(fromCdsSArray, cList);
  ASSERT_EQ(fromStdVector, cList);
  ASSERT_EQ(fromStdArray, cList);
  ASSERT_EQ(fromCArray, cList);
  ASSERT_EQ(fromCArrayUnsized, cList);
  ASSERT_EQ(fromInitializerList, cList);
}

TEST(ArrayRefTest, eq) {
  // Another set of temporary extensions
  std::initializer_list<int> const& eTemp1 = {1, 5, 4, 2, 3};
  std::initializer_list<int> const& eTemp2 = {1, 5, 4, 2, 3};
  std::initializer_list<int> const& eTemp3 = {1, 5, 4, 2};
  std::initializer_list<int> const& eTemp4 = {1, 5, 4, 2, 3, 6};
  std::initializer_list<int> const& eTemp5 = {1, 5, 2, 4, 3};

  ArrayRef base = eTemp1;
  ArrayRef baseEq = eTemp2;
  ArrayRef baseDifSh = eTemp3;
  ArrayRef baseDifLg = eTemp4;
  ArrayRef baseDif = eTemp5;

  ASSERT_EQ(base, base);

  ASSERT_EQ(base, baseEq);
  ASSERT_NE(base, baseDifSh);
  ASSERT_NE(base, baseDifLg);
  ASSERT_NE(base, baseDif);

  ASSERT_EQ(baseEq, base);
  ASSERT_NE(baseDifSh, base);
  ASSERT_NE(baseDifLg, base);
  ASSERT_NE(baseDif, base);

  ASSERT_EQ(base, eTemp2);
  ASSERT_NE(base, eTemp3);
  ASSERT_NE(base, eTemp4);
  ASSERT_NE(base, eTemp5);

  ASSERT_EQ(eTemp2, base);
  ASSERT_NE(eTemp3, base);
  ASSERT_NE(eTemp4, base);
  ASSERT_NE(eTemp5, base);

  ASSERT_EQ(base, cds::Array(eTemp2));
  ASSERT_NE(base, cds::Array(eTemp3));
  ASSERT_NE(base, cds::Array(eTemp4));
  ASSERT_NE(base, cds::Array(eTemp5));

  ASSERT_EQ(cds::Array(eTemp2), base);
  ASSERT_NE(cds::Array(eTemp3), base);
  ASSERT_NE(cds::Array(eTemp4), base);
  ASSERT_NE(cds::Array(eTemp5), base);

  using SA_Ctor = cds::StaticArray<int, 5>;
  using SA_CtorLt = cds::StaticArray<int, 4>;
  using SA_CtorGt = cds::StaticArray<int, 6>;
  ASSERT_EQ(base, SA_Ctor(eTemp2));
  ASSERT_NE(base, SA_CtorLt(eTemp3));
  ASSERT_NE(base, SA_CtorGt(eTemp4));
  ASSERT_NE(base, SA_Ctor(eTemp5));

  ASSERT_EQ(SA_Ctor(eTemp2), base);
  ASSERT_NE(SA_CtorLt(eTemp3), base);
  ASSERT_NE(SA_CtorGt(eTemp4), base);
  ASSERT_NE(SA_Ctor(eTemp5), base);

  ASSERT_EQ(base, std::vector<int>(eTemp2));
  ASSERT_NE(base, std::vector<int>(eTemp3));
  ASSERT_NE(base, std::vector<int>(eTemp4));
  ASSERT_NE(base, std::vector<int>(eTemp5));

  ASSERT_EQ(std::vector<int>(eTemp2), base);
  ASSERT_NE(std::vector<int>(eTemp3), base);
  ASSERT_NE(std::vector<int>(eTemp4), base);
  ASSERT_NE(std::vector<int>(eTemp5), base);

  std::array<int, 5> baseEqArr = {1, 5, 4, 2, 3};
  std::array<int, 4> baseEqLeArr = {1, 5, 4, 2};
  std::array<int, 6> baseEqGrArr = {1, 5, 4, 2, 3, 6};
  std::array<int, 5> baseEqDArr = {1, 5, 2, 4, 3};
  ASSERT_EQ(base, baseEqArr);
  ASSERT_NE(base, baseEqLeArr);
  ASSERT_NE(base, baseEqGrArr);
  ASSERT_NE(base, baseEqDArr);

  ASSERT_EQ(baseEqArr, base);
  ASSERT_NE(baseEqLeArr, base);
  ASSERT_NE(baseEqGrArr, base);
  ASSERT_NE(baseEqDArr, base);
}

TEST(ArrayRefTest, assign) {
  ArrayRef<int const> base;
  ASSERT_EQ(base, cds::Array<int>());

  std::initializer_list<int> const& eTemp = {1, 2, 3, 4, 5};
  std::initializer_list<int> const& eTemp1 = {1, 2, 3, 4, 6};
  std::initializer_list<int> const& eTemp2 = {1, 2, 3, 4, 7};
  base = eTemp;
  ASSERT_EQ(base, eTemp);

  base = ArrayRef(eTemp1);
  ASSERT_EQ(base, eTemp1);

  ArrayRef moved(eTemp2);
  base = std::move(moved);
  ASSERT_EQ(base, eTemp2);

  // TODO: int const -> int once RandomAccessIterable for cds::Array is fixed
  // FIXME: Array<int const> not possible due to no decay done at allocation.
  // Come back to this later
  //  cds::Array<int const> baseArr {1, 5, 4, 2, 3};
  //  base = baseArr;
  //  ASSERT_EQ(base, baseArr);

  // FIXME And also applies for staticarray init by InitList
  //  cds::StaticArray<int const, 5> baseSArr {1, 7, 8, 2, 3};
  //  base = baseSArr;
  //  ASSERT_EQ(base, baseSArr);
  // PH:

  ArrayRef<int> ncBase;
  cds::Array<int> baseArr {1, 5, 4, 2, 3};
  ncBase = baseArr;
  ASSERT_EQ(ncBase, baseArr);

  cds::StaticArray<int, 5> baseSArr {1, 7, 8, 2, 3};
  ncBase = baseSArr;
  ASSERT_EQ(ncBase, baseSArr);

  std::vector<int> baseVec = {9, 3, 4, 1, 5};
  base = baseVec;
  ASSERT_EQ(base, baseVec);

  std::array<int, 5> baseStdArr = {9, 1, 3, 4, 5};
  base = baseStdArr;
  ASSERT_EQ(base, baseStdArr);

  int cArr[5] = {5, 4, 3, 2, 1};
  ncBase = cArr;
  ASSERT_EQ(ncBase, cArr);
}

TEST(ArrayRefTest, empty) {
  ArrayRef<int> empty;
  std::initializer_list<int> const& eTemp = {1, 2, 3, 4, 5};
  ArrayRef nEmpty = eTemp;

  ASSERT_TRUE(empty.empty());
  ASSERT_FALSE(nEmpty.empty());
  ASSERT_TRUE(nEmpty);
  ASSERT_FALSE(empty);
}

TEST(ArrayRefTest, clear) {
  ArrayRef<int> empty;
  ASSERT_FALSE(empty);
  empty.clear();
  ASSERT_FALSE(empty);
  std::vector<int> buf = {1, 4, 3};
  empty = buf;
  ASSERT_TRUE(empty);
  empty.clear();
  ASSERT_FALSE(empty);
}

TEST(ArrayRefTest, takeFront) {
  std::initializer_list<int> const& eTemp1 = {1, 2, 3, 4, 5};
  ArrayRef r = eTemp1;
  ASSERT_EQ(r, eTemp1);

  ASSERT_EQ(r.takeFront(10), eTemp1);
  ASSERT_EQ(r.takeFront(5), eTemp1);

  std::initializer_list<int> const& eTemp2 = {1, 2, 3, 4};
  ASSERT_EQ(r.takeFront(4), eTemp2);

  std::initializer_list<int> const& eTemp3 = {1, 2};
  ASSERT_EQ(r.takeFront(2), eTemp3);

  ASSERT_TRUE(r.takeFront(1));
  ASSERT_FALSE(r.takeFront(0));

  ArrayRef<int> empty;
  ASSERT_FALSE(empty.takeFront(0));
  ASSERT_FALSE(empty.takeFront(4));
  ASSERT_FALSE(empty.takeFront(5));
  ASSERT_FALSE(empty.takeFront(6));
  ASSERT_FALSE(empty.takeFront(10));
}

TEST(ArrayRefTest, takeFrontConst) {
  std::initializer_list<int> const& eTemp1 = {1, 2, 3, 4, 5};
  ArrayRef const r = eTemp1;
  ASSERT_EQ(r, eTemp1);

  ASSERT_EQ(r.takeFront(10), eTemp1);
  ASSERT_EQ(r.takeFront(5), eTemp1);

  std::initializer_list<int> const& eTemp2 = {1, 2, 3, 4};
  ASSERT_EQ(r.takeFront(4), eTemp2);

  std::initializer_list<int> const& eTemp3 = {1, 2};
  ASSERT_EQ(r.takeFront(2), eTemp3);

  ASSERT_TRUE(r.takeFront(1));
  ASSERT_FALSE(r.takeFront(0));

  ArrayRef<int> const empty;
  ASSERT_FALSE(empty.takeFront(0));
  ASSERT_FALSE(empty.takeFront(4));
  ASSERT_FALSE(empty.takeFront(5));
  ASSERT_FALSE(empty.takeFront(6));
  ASSERT_FALSE(empty.takeFront(10));
}

TEST(ArrayRefTest, takeBack) {
  std::initializer_list<int> const& eTemp1 = {1, 2, 3, 4, 5};
  ArrayRef r = eTemp1;
  ASSERT_EQ(r, eTemp1);

  ASSERT_EQ(r.takeBack(10), eTemp1);
  ASSERT_EQ(r.takeBack(6), eTemp1);
  ASSERT_EQ(r.takeBack(5), eTemp1);

  std::initializer_list<int> const& eTemp2 = {3, 4, 5};
  std::initializer_list<int> const& eTemp3 = {4, 5};
  std::initializer_list<int> const& eTemp4 = {2, 3};

  ASSERT_EQ(r.takeBack(3), eTemp2);
  ASSERT_EQ(r.takeBack(2), eTemp3);
  ASSERT_EQ(r.takeFront(3).takeBack(2), eTemp4);
  ASSERT_TRUE(r.takeBack(1));
  ASSERT_FALSE(r.takeBack(0));

  ArrayRef<int> empty;
  ASSERT_FALSE(empty.takeBack(0));
  ASSERT_FALSE(empty.takeBack(4));
  ASSERT_FALSE(empty.takeBack(5));
  ASSERT_FALSE(empty.takeBack(6));
  ASSERT_FALSE(empty.takeBack(10));
}

TEST(ArrayRefTest, takeBackConst) {
  std::initializer_list<int> const& eTemp1 = {1, 2, 3, 4, 5};
  ArrayRef const r = eTemp1;
  ASSERT_EQ(r, eTemp1);

  ASSERT_EQ(r.takeBack(10), eTemp1);
  ASSERT_EQ(r.takeBack(6), eTemp1);
  ASSERT_EQ(r.takeBack(5), eTemp1);

  std::initializer_list<int> const& eTemp2 = {3, 4, 5};
  std::initializer_list<int> const& eTemp3 = {4, 5};
  std::initializer_list<int> const& eTemp4 = {2, 3};

  ASSERT_EQ(r.takeBack(3), eTemp2);
  ASSERT_EQ(r.takeBack(2), eTemp3);
  ASSERT_EQ(r.takeFront(3).takeBack(2), eTemp4);
  ASSERT_TRUE(r.takeBack(1));
  ASSERT_FALSE(r.takeBack(0));

  ArrayRef<int> const empty;
  ASSERT_FALSE(empty.takeBack(0));
  ASSERT_FALSE(empty.takeBack(4));
  ASSERT_FALSE(empty.takeBack(5));
  ASSERT_FALSE(empty.takeBack(6));
  ASSERT_FALSE(empty.takeBack(10));
}

TEST(ArrayRefTest, dropFront) {
  std::initializer_list<int> const& eTemp1 = {1, 2, 3, 4, 5};
  ArrayRef r = eTemp1;
  ASSERT_EQ(r, eTemp1);
  ASSERT_EQ(r.dropFront(0), eTemp1);

  std::initializer_list<int> const& eTemp2 = {3, 4, 5};
  std::initializer_list<int> const& eTemp3 = {5};
  std::initializer_list<int> const& eTemp4 = {3, 4};
  ASSERT_EQ(r.dropFront(2), eTemp2);
  ASSERT_EQ(r.dropFront(4), eTemp3);
  ASSERT_EQ(r.dropFront(2).takeFront(2), eTemp4);

  ASSERT_TRUE(r.dropFront(0));
  ASSERT_TRUE(r.dropFront(4));
  ASSERT_FALSE(r.dropFront(5));
  ASSERT_FALSE(r.dropFront(6));
  ASSERT_FALSE(r.dropFront(10));

  ArrayRef<int> empty;
  ASSERT_FALSE(empty.dropFront(0));
  ASSERT_FALSE(empty.dropFront(4));
  ASSERT_FALSE(empty.dropFront(5));
  ASSERT_FALSE(empty.dropFront(6));
  ASSERT_FALSE(empty.dropFront(10));
}

TEST(ArrayRefTest, dropFrontConst) {
  std::initializer_list<int> const& eTemp1 = {1, 2, 3, 4, 5};
  ArrayRef const r = eTemp1;
  ASSERT_EQ(r, eTemp1);
  ASSERT_EQ(r.dropFront(0), eTemp1);

  std::initializer_list<int> const& eTemp2 = {3, 4, 5};
  std::initializer_list<int> const& eTemp3 = {5};
  std::initializer_list<int> const& eTemp4 = {3, 4};
  ASSERT_EQ(r.dropFront(2), eTemp2);
  ASSERT_EQ(r.dropFront(4), eTemp3);
  ASSERT_EQ(r.dropFront(2).takeFront(2), eTemp4);

  ASSERT_TRUE(r.dropFront(0));
  ASSERT_TRUE(r.dropFront(4));
  ASSERT_FALSE(r.dropFront(5));
  ASSERT_FALSE(r.dropFront(6));
  ASSERT_FALSE(r.dropFront(10));

  ArrayRef<int> const empty;
  ASSERT_FALSE(empty.dropFront(0));
  ASSERT_FALSE(empty.dropFront(4));
  ASSERT_FALSE(empty.dropFront(5));
  ASSERT_FALSE(empty.dropFront(6));
  ASSERT_FALSE(empty.dropFront(10));
}

TEST(ArrayRefTest, dropBack) {
  std::initializer_list<int> const& eTemp1 = {1, 2, 3, 4, 5};
  ArrayRef r = eTemp1;
  ASSERT_EQ(r, eTemp1);
  ASSERT_EQ(r.dropBack(0), eTemp1);

  std::initializer_list<int> const& eTemp2 = {1, 2, 3};
  std::initializer_list<int> const& eTemp3 = {1};
  std::initializer_list<int> const& eTemp4 = {3, 4};
  ASSERT_EQ(r.dropBack(2), eTemp2);
  ASSERT_EQ(r.dropBack(4), eTemp3);
  ASSERT_EQ(r.dropFront(2).dropBack(1), eTemp4);

  ASSERT_TRUE(r.dropBack(0));
  ASSERT_TRUE(r.dropBack(4));
  ASSERT_FALSE(r.dropBack(5));
  ASSERT_FALSE(r.dropBack(6));
  ASSERT_FALSE(r.dropBack(10));

  ArrayRef<int> empty;
  ASSERT_FALSE(empty.dropBack(0));
  ASSERT_FALSE(empty.dropBack(4));
  ASSERT_FALSE(empty.dropBack(5));
  ASSERT_FALSE(empty.dropBack(6));
  ASSERT_FALSE(empty.dropBack(10));
}

TEST(ArrayRefTest, dropBackConst) {
  std::initializer_list<int> const& eTemp1 = {1, 2, 3, 4, 5};
  ArrayRef const r = eTemp1;
  ASSERT_EQ(r, eTemp1);
  ASSERT_EQ(r.dropBack(0), eTemp1);

  std::initializer_list<int> const& eTemp2 = {1, 2, 3};
  std::initializer_list<int> const& eTemp3 = {1};
  std::initializer_list<int> const& eTemp4 = {3, 4};
  ASSERT_EQ(r.dropBack(2), eTemp2);
  ASSERT_EQ(r.dropBack(4), eTemp3);
  ASSERT_EQ(r.dropFront(2).dropBack(1), eTemp4);

  ASSERT_TRUE(r.dropBack(0));
  ASSERT_TRUE(r.dropBack(4));
  ASSERT_FALSE(r.dropBack(5));
  ASSERT_FALSE(r.dropBack(6));
  ASSERT_FALSE(r.dropBack(10));

  ArrayRef<int> const empty;
  ASSERT_FALSE(empty.dropBack(0));
  ASSERT_FALSE(empty.dropBack(4));
  ASSERT_FALSE(empty.dropBack(5));
  ASSERT_FALSE(empty.dropBack(6));
  ASSERT_FALSE(empty.dropBack(10));
}

TEST(ArrayRefTest, idx) {
  std::vector<int> cont = {1, 2, 3, 4, 5};
  ArrayRef r = cont;
  ArrayRef const cr = cont;

  ASSERT_EQ(r[0], cont[0]);
  ASSERT_EQ(r[1], cont[1]);
  ASSERT_EQ(r[2], cont[2]);
  ASSERT_EQ(r[3], cont[3]);
  ASSERT_EQ(r[4], cont[4]);

  ASSERT_EQ(cr[0], cont[0]);
  ASSERT_EQ(cr[1], cont[1]);
  ASSERT_EQ(cr[2], cont[2]);
  ASSERT_EQ(cr[3], cont[3]);
  ASSERT_EQ(cr[4], cont[4]);

  r[0] = 6;
  ASSERT_EQ(cont[0], 6);
  r[3] = 7;
  ASSERT_EQ(cont[3], 7);
  ASSERT_EQ(cont[1], 2);
  ASSERT_EQ(cont[0], 6);
  ASSERT_EQ(cont[2], 3);
  ASSERT_EQ(cont[4], 5);

  ASSERT_EQ(cr[0], r[0]);
  ASSERT_EQ(cr[1], r[1]);
  ASSERT_EQ(cr[2], r[2]);
  ASSERT_EQ(cr[3], r[3]);
  ASSERT_EQ(cr[4], r[4]);
}

TEST(ArrayRefTest, util) {
  std::vector<int> cont = {1, 2, 3, 4, 5};
  ArrayRef r = cont;
  ArrayRef const cr = cont;

  ASSERT_EQ(r.data(), cont.data());
  ASSERT_EQ(cr.data(), cont.data());
  ASSERT_EQ(r.data(), cr.data());

  ASSERT_EQ(r.size(), cont.size());
  ASSERT_EQ(r.size(), cr.size());
  ASSERT_EQ(cr.size(), cont.size());
}

TEST(ArrayRefTest, iter) {
  std::vector<int> cont = {1, 2, 3};

  ArrayRef<int> empty;
  ASSERT_EQ(empty.begin(), empty.end());
  ASSERT_EQ(empty.cbegin(), empty.cend());
  ASSERT_EQ(empty.rbegin(), empty.rend());
  ASSERT_EQ(empty.crbegin(), empty.crend());

  ArrayRef r = cont;
  auto fwd = r.begin();
  ASSERT_NE(fwd, r.end());

  ASSERT_EQ(*fwd, 1);
  fwd++;
  ASSERT_NE(fwd, r.end());

  ASSERT_EQ(*fwd, 2);
  fwd++;
  ASSERT_NE(fwd, r.end());

  ASSERT_EQ(*fwd, 3);
  fwd++;
  ASSERT_EQ(fwd, r.end());


  auto fwdConst = r.cbegin();
  ASSERT_NE(fwdConst, r.cend());
  ASSERT_EQ(*fwdConst, 1);
  fwdConst++;
  ASSERT_NE(fwdConst, r.cend());

  ASSERT_EQ(*fwdConst, 2);
  fwdConst++;
  ASSERT_NE(fwdConst, r.cend());

  ASSERT_EQ(*fwdConst, 3);
  fwdConst++;
  ASSERT_EQ(fwdConst, r.cend());


  auto bwd = r.rbegin();
  ASSERT_NE(bwd, r.rend());
  ASSERT_EQ(*bwd, 3);
  bwd++;
  ASSERT_NE(bwd, r.rend());

  ASSERT_EQ(*bwd, 2);
  bwd++;
  ASSERT_NE(bwd, r.rend());

  ASSERT_EQ(*bwd, 1);
  bwd++;
  ASSERT_EQ(bwd, r.rend());


  auto bwdConst = r.crbegin();
  ASSERT_NE(bwdConst, r.crend());
  ASSERT_EQ(*bwdConst, 3);
  bwdConst++;
  ASSERT_NE(bwdConst, r.crend());

  ASSERT_EQ(*bwdConst, 2);
  bwdConst++;
  ASSERT_NE(bwdConst, r.crend());

  ASSERT_EQ(*bwdConst, 1);
  bwdConst++;
  ASSERT_EQ(bwdConst, r.crend());
}

TEST(ArrayRefTest, iterConst) {
  std::vector<int> cont = {1, 2, 3};

  ArrayRef<int> const empty;
  ASSERT_EQ(empty.begin(), empty.end());
  ASSERT_EQ(empty.cbegin(), empty.cend());
  ASSERT_EQ(empty.rbegin(), empty.rend());
  ASSERT_EQ(empty.crbegin(), empty.crend());

  ArrayRef const r = cont;
  auto fwd = r.begin();
  ASSERT_NE(fwd, r.end());
  ASSERT_EQ(*fwd, 1);
  fwd++;
  ASSERT_NE(fwd, r.end());

  ASSERT_EQ(*fwd, 2);
  fwd++;
  ASSERT_NE(fwd, r.end());

  ASSERT_EQ(*fwd, 3);
  fwd++;
  ASSERT_EQ(fwd, r.end());


  auto fwdConst = r.cbegin();
  ASSERT_NE(fwdConst, r.cend());
  ASSERT_EQ(*fwdConst, 1);
  fwdConst++;
  ASSERT_NE(fwdConst, r.cend());

  ASSERT_EQ(*fwdConst, 2);
  fwdConst++;
  ASSERT_NE(fwdConst, r.cend());

  ASSERT_EQ(*fwdConst, 3);
  fwdConst++;
  ASSERT_EQ(fwdConst, r.cend());


  auto bwd = r.rbegin();
  ASSERT_NE(bwd, r.rend());
  ASSERT_EQ(*bwd, 3);
  bwd++;
  ASSERT_NE(bwd, r.rend());

  ASSERT_EQ(*bwd, 2);
  bwd++;
  ASSERT_NE(bwd, r.rend());

  ASSERT_EQ(*bwd, 1);
  bwd++;
  ASSERT_EQ(bwd, r.rend());


  auto bwdConst = r.crbegin();
  ASSERT_NE(bwdConst, r.crend());
  ASSERT_EQ(*bwdConst, 3);
  bwdConst++;
  ASSERT_NE(bwdConst, r.crend());

  ASSERT_EQ(*bwdConst, 2);
  bwdConst++;
  ASSERT_NE(bwdConst, r.crend());

  ASSERT_EQ(*bwdConst, 1);
  bwdConst++;
  ASSERT_EQ(bwdConst, r.crend());
}
