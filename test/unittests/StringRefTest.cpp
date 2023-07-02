//
// Created by Vlad-Andrei Loghin on 26.06.23.
//

#include <gtest/gtest.h>
#include <lang/string/StringRef.hpp>

namespace {
using namespace cds;
using namespace age;

auto comparability(auto const& l, auto const& r) {
  int count = 0;
  if (l == r) {
    ++count;
  }

  if (l < r) {
    ++count;
  }

  if (l > r) {
    ++count;
  }

  return count == 1;
}
} // namespace

TEST(StringRefTest, ConstructionDataSize) {
  String cdsStr = "CdsStr";
  StringView cdsView = "CdsView";
  std::string stdStr = "StdStr";
  std::string_view stdView = "StdView";
  char const* cStr = "CStr";

  StringRef r1;
  StringRef r2 = cdsStr;
  StringRef r3 = cdsView;
  StringRef r4 = stdStr;
  StringRef r5 = stdView;
  StringRef r6 = cStr;
  StringRef r7 = r2;
  StringRef moved = cdsStr;
  StringRef r8 = std::move(moved);
  StringRef r9 = {"CStrLen", 7};

  ASSERT_EQ(r1.data(), nullptr);
  ASSERT_EQ(r1.size(), 0u);

  ASSERT_STREQ(r2.data(), "CdsStr");
  ASSERT_EQ(r2.size(), 6u);

  ASSERT_STREQ(r3.data(), "CdsView");
  ASSERT_EQ(r3.size(), 7u);

  ASSERT_STREQ(r4.data(), "StdStr");
  ASSERT_EQ(r4.size(), 6u);

  ASSERT_STREQ(r5.data(), "StdView");
  ASSERT_EQ(r5.size(), 7u);

  ASSERT_STREQ(r6.data(), "CStr");
  ASSERT_EQ(r6.size(), 4u);

  ASSERT_STREQ(r7.data(), "CdsStr");
  ASSERT_EQ(r7.size(), 6u);

  ASSERT_STREQ(r8.data(), "CdsStr");
  ASSERT_EQ(r8.size(), 6u);

  ASSERT_STREQ(r9.data(), "CStrLen");
  ASSERT_EQ(r9.size(), 7u);
}

TEST(StringRefTest, Assign) {
  StringRef r;

  String cdsStr = "CdsStr";
  StringView cdsView = "CdsView";
  std::string stdStr = "StdStr";
  std::string_view stdView = "StdView";
  char const* cStr = "CStr";
  StringRef rc = cdsStr;
  StringRef rm = cdsStr;

  r = rc;
  ASSERT_STREQ(r.data(), "CdsStr");
  ASSERT_EQ(r.size(), 6u);

  r = std::move(rm);
  ASSERT_STREQ(r.data(), "CdsStr");
  ASSERT_EQ(r.size(), 6u);

  r = cdsStr;
  ASSERT_STREQ(r.data(), "CdsStr");
  ASSERT_EQ(r.size(), 6u);

  r = cdsView;
  ASSERT_STREQ(r.data(), "CdsView");
  ASSERT_EQ(r.size(), 7u);

  r = stdView;
  ASSERT_STREQ(r.data(), "StdView");
  ASSERT_EQ(r.size(), 7u);

  r = stdStr;
  ASSERT_STREQ(r.data(), "StdStr");
  ASSERT_EQ(r.size(), 6u);

  r = cStr;
  ASSERT_STREQ(r.data(), "CStr");
  ASSERT_EQ(r.size(), 4u);

  r = r;
  ASSERT_STREQ(r.data(), "CStr");
  ASSERT_EQ(r.size(), 4u);

  r = std::move(r);
  ASSERT_STREQ(r.data(), "CStr");
  ASSERT_EQ(r.size(), 4u);
}

TEST(StringRefTest, empty) {
  StringRef sr;
  StringRef nSr = nullptr;
  StringRef eSr = "";
  String eCdsStr;
  StringView eCdsView;
  std::string eStdStr;
  std::string_view eStdView;
  char const* eCStr = "";

  ASSERT_FALSE(sr);
  ASSERT_FALSE(nSr);
  ASSERT_FALSE(eSr);
  ASSERT_FALSE(StringRef(eCdsStr));
  ASSERT_FALSE(StringRef(eCdsView));
  ASSERT_FALSE(StringRef(eStdStr));
  ASSERT_FALSE(StringRef(eStdView));
  ASSERT_FALSE(StringRef(eCStr));

  StringRef nsr = "abc";
  StringRef nsrcopy = nsr;
  StringRef move = nsr;
  StringRef nsrmove = std::move(move);
  String neSr = "abc";
  StringView neSVr = "abcd";
  std::string nestd = "abc";
  std::string_view nestrv = "agbc";
  char const* neCstr = "abc";

  ASSERT_TRUE(nsr);
  ASSERT_TRUE(nsrcopy);
  ASSERT_TRUE(nsrmove);
  ASSERT_TRUE(StringRef(neSr));
  ASSERT_TRUE(StringRef(neSVr));
  ASSERT_TRUE(StringRef(nestd));
  ASSERT_TRUE(StringRef(nestrv));
  ASSERT_TRUE(StringRef(neCstr));
}

TEST(StringRefTest, StringRefConv) {
  StringRef r = "abc";
  StringView v = r;
  String s = "def";
  ASSERT_EQ(s + v, "defabc");
}

TEST(StringRefTest, clear) {
  StringRef ref = "Abcd";
  ref.clear();
  ASSERT_EQ(ref.data(), nullptr);
  ASSERT_EQ(ref.size(), 0u);
}

TEST(StringRefTest, dropFront) {
  StringRef ref = "abcd";

  ref = ref.dropFront(0);
  ASSERT_STREQ(ref.data(), "abcd");
  ASSERT_EQ(ref.size(), 4u);

  ref = ref.dropFront(1);
  ASSERT_STREQ(ref.data(), "bcd");
  ASSERT_EQ(ref.size(), 3u);

  ref = ref.dropFront(2);
  ASSERT_STREQ(ref.data(), "d");
  ASSERT_EQ(ref.size(), 1u);

  ref = ref.dropFront(3);
  ASSERT_EQ(ref.data(), nullptr);
  ASSERT_EQ(ref.size(), 0u);

  ref = ref.dropFront(1);
  ASSERT_EQ(ref.data(), nullptr);
  ASSERT_EQ(ref.size(), 0u);

  ref = ref.dropFront(0);
  ASSERT_EQ(ref.data(), nullptr);
  ASSERT_EQ(ref.size(), 0u);
}

TEST(StringRefTest, dropBack) {
  StringRef ref = "abcd";

  ref = ref.dropBack(0);
  ASSERT_STREQ(ref.data(), "abcd");
  ASSERT_EQ(ref.size(), 4u);

  ref = ref.dropBack(1);
  ASSERT_STREQ(ref.data(), "abcd");
  ASSERT_EQ(ref.size(), 3u);

  ref = ref.dropBack(2);
  ASSERT_STREQ(ref.data(), "abcd");
  ASSERT_EQ(ref.size(), 1u);

  ref = ref.dropBack(3);
  ASSERT_EQ(ref.data(), nullptr);
  ASSERT_EQ(ref.size(), 0u);

  ref = ref.dropBack(1);
  ASSERT_EQ(ref.data(), nullptr);
  ASSERT_EQ(ref.size(), 0u);

  ref = ref.dropBack(0);
  ASSERT_EQ(ref.data(), nullptr);
  ASSERT_EQ(ref.size(), 0u);
}

TEST(StringRefTest, takeFront) {
  StringRef ref = "abcd";

  ref = ref.takeFront(5);
  ASSERT_STREQ(ref.data(), "abcd");
  ASSERT_EQ(ref.size(), 4u);

  ref = ref.takeFront(3);
  ASSERT_STREQ(ref.data(), "abcd");
  ASSERT_EQ(ref.size(), 3u);

  ref = ref.takeFront(1);
  ASSERT_STREQ(ref.data(), "abcd");
  ASSERT_EQ(ref.size(), 1u);

  ref = ref.takeFront(3u);
  ASSERT_EQ(ref.data(), "abcd");
  ASSERT_EQ(ref.size(), 1u);

  ref = ref.takeFront(0u);
  ASSERT_EQ(ref.data(), "abcd");
  ASSERT_EQ(ref.size(), 0u);

  ref = ref.takeFront(2u);
  ASSERT_EQ(ref.data(), "abcd");
  ASSERT_EQ(ref.size(), 0u);
}

TEST(StringRefTest, takeBack) {
  StringRef ref = "abcd";

  ref = ref.takeBack(5);
  ASSERT_STREQ(ref.data(), "abcd");
  ASSERT_EQ(ref.size(), 4u);

  ref = ref.takeBack(3);
  ASSERT_STREQ(ref.data(), "bcd");
  ASSERT_EQ(ref.size(), 3u);

  ref = ref.takeBack(1);
  ASSERT_STREQ(ref.data(), "d");
  ASSERT_EQ(ref.size(), 1u);

  ref = ref.takeBack(3u);
  ASSERT_STREQ(ref.data(), "d");
  ASSERT_EQ(ref.size(), 1u);

  ref = ref.takeBack(0u);
  ASSERT_STREQ(ref.data(), "");
  ASSERT_EQ(ref.size(), 0u);

  ref = ref.takeBack(2u);
  ASSERT_STREQ(ref.data(), "");
  ASSERT_EQ(ref.size(), 0u);
}

TEST(StringRefTest, shrink) {
  StringRef ref = "abcd";

  ref.shrink(0);
  ASSERT_STREQ(ref.data(), "abcd");
  ASSERT_EQ(ref.size(), 4u);

  ref.shrink(1);
  ASSERT_STREQ(ref.data(), "abcd");
  ASSERT_EQ(ref.size(), 3u);

  ref.shrink(2);
  ASSERT_STREQ(ref.data(), "abcd");
  ASSERT_EQ(ref.size(), 1u);

  ref.shrink(3);
  ASSERT_EQ(ref.data(), nullptr);
  ASSERT_EQ(ref.size(), 0u);

  ref.shrink(1);
  ASSERT_EQ(ref.data(), nullptr);
  ASSERT_EQ(ref.size(), 0u);

  ref.shrink(0);
  ASSERT_EQ(ref.data(), nullptr);
  ASSERT_EQ(ref.size(), 0u);
}

TEST(StringRefTest, find) {
  StringRef empty;
  StringRef emptyRef = "";
  StringRef ref = "abcd";

  auto n = StringRef::npos;
  ASSERT_EQ(empty.find('a'), n);
  ASSERT_EQ(emptyRef.find('a'), n);
  ASSERT_EQ(ref.find('a'), 0);
  ASSERT_EQ(ref.find('b'), 1);
  ASSERT_EQ(ref.find('c'), 2);
  ASSERT_EQ(ref.find('d'), 3);
  ASSERT_EQ(ref.find('e'), n);
}

TEST(StringRefTest, sub) {
  StringRef empty;
  StringRef emptyRef = "";
  StringRef ref = "abcd";

  ASSERT_EQ(empty.sub(0, 0).data(), nullptr);
  ASSERT_EQ(empty.sub(0, 0).size(), 0u);

  ASSERT_EQ(empty.sub(1, 0).data(), nullptr);
  ASSERT_EQ(empty.sub(1, 0).size(), 0u);

  ASSERT_EQ(empty.sub(0, 1).data(), nullptr);
  ASSERT_EQ(empty.sub(0, 1).size(), 0u);

  ASSERT_EQ(empty.sub(1, 1).data(), nullptr);
  ASSERT_EQ(empty.sub(1, 1).size(), 0u);

  ASSERT_EQ(emptyRef.sub(0, 0).data(), nullptr);
  ASSERT_EQ(emptyRef.sub(0, 0).size(), 0u);

  ASSERT_EQ(emptyRef.sub(1, 0).data(), nullptr);
  ASSERT_EQ(emptyRef.sub(1, 0).size(), 0u);

  ASSERT_EQ(emptyRef.sub(0, 1).data(), nullptr);
  ASSERT_EQ(emptyRef.sub(0, 1).size(), 0u);

  ASSERT_EQ(emptyRef.sub(1, 1).data(), nullptr);
  ASSERT_EQ(emptyRef.sub(1, 1).size(), 0u);

  ASSERT_STREQ(ref.sub(0, 4).data(), "abcd");
  ASSERT_EQ(ref.sub(0, 4).size(), 4u);

  ASSERT_STREQ(ref.sub(1, 3).data(), "bcd");
  ASSERT_EQ(ref.sub(1, 3).size(), 3u);

  ASSERT_STREQ(ref.sub(1, 4).data(), "bcd");
  ASSERT_EQ(ref.sub(1, 4).size(), 3u);

  ASSERT_STREQ(ref.sub(0, 3).data(), "abcd");
  ASSERT_EQ(ref.sub(0, 3).size(), 3u);

  ASSERT_STREQ(ref.sub(0, 5).data(), "abcd");
  ASSERT_EQ(ref.sub(0, 5).size(), 4u);

  ASSERT_STREQ(ref.sub(0, 5).data(), "abcd");
  ASSERT_EQ(ref.sub(0, 5).size(), 4u);

  ASSERT_EQ(ref.sub(4, 0).data(), nullptr);
  ASSERT_EQ(ref.sub(4, 0).size(), 0u);

  ASSERT_STREQ(ref.sub(5, 0).data(), nullptr);
  ASSERT_EQ(ref.sub(5, 0).size(), 0u);
}

TEST(StringRefTest, add) {
  StringRef l = "abc";
  StringRef r = "def";

  ASSERT_EQ(l + r, "abcdef");
  ASSERT_EQ(l + "def", "abcdef");
  ASSERT_EQ(l + String("def"), "abcdef");
  ASSERT_EQ(l + StringView("def"), "abcdef");
  ASSERT_EQ(l + std::string("def"), "abcdef");
  ASSERT_EQ(l + std::string_view("def"), "abcdef");

  ASSERT_EQ(String("def") + l, "defabc");
  ASSERT_EQ(StringView("def") + l, "defabc");
  ASSERT_EQ(std::string("def") + l, "defabc");
  ASSERT_EQ(std::string_view("def") + l, "defabc");
}

TEST(StringRef, cmpSR) {
  StringRef r = "abcd";

  StringRef e = "abcd";
  StringRef l = "abc";
  StringRef g = "abcde";

  ASSERT_TRUE(comparability(r, e));
  ASSERT_TRUE(comparability(r, l));
  ASSERT_TRUE(comparability(r, g));

  ASSERT_EQ(r, e);
  ASSERT_LT(l, r);
  ASSERT_GT(r, l);
  ASSERT_LT(r, g);
  ASSERT_GT(g, r);
  ASSERT_LE(l, r);
  ASSERT_LE(r, g);
  ASSERT_GE(g, r);
  ASSERT_GE(r, l);
  ASSERT_LE(r, e);
  ASSERT_LE(e, r);
  ASSERT_GE(r, e);
  ASSERT_GE(e, r);
}

TEST(StringRef, cmpCdsStr) {
  StringRef r = "abcd";

  String e = "abcd";
  String l = "abc";
  String g = "abcde";

  ASSERT_TRUE(comparability(r, e));
  ASSERT_TRUE(comparability(r, l));
  ASSERT_TRUE(comparability(r, g));

  ASSERT_EQ(r, e);
  ASSERT_LT(l, r);
  ASSERT_GT(r, l);
  ASSERT_LT(r, g);
  ASSERT_GT(g, r);
  ASSERT_LE(l, r);
  ASSERT_LE(r, g);
  ASSERT_GE(g, r);
  ASSERT_GE(r, l);
  ASSERT_LE(r, e);
  ASSERT_LE(e, r);
  ASSERT_GE(r, e);
  ASSERT_GE(e, r);
}

TEST(StringRef, cmpCdsView) {
  StringRef r = "abcd";

  StringView e = "abcd";
  StringView l = "abc";
  StringView g = "abcde";

  ASSERT_TRUE(comparability(r, e));
  ASSERT_TRUE(comparability(r, l));
  ASSERT_TRUE(comparability(r, g));

  ASSERT_EQ(r, e);
  ASSERT_LT(l, r);
  ASSERT_GT(r, l);
  ASSERT_LT(r, g);
  ASSERT_GT(g, r);
  ASSERT_LE(l, r);
  ASSERT_LE(r, g);
  ASSERT_GE(g, r);
  ASSERT_GE(r, l);
  ASSERT_LE(r, e);
  ASSERT_LE(e, r);
  ASSERT_GE(r, e);
  ASSERT_GE(e, r);
}

TEST(StringRef, cmpStdStr) {
  StringRef r = "abcd";

  std::string e = "abcd";
  std::string l = "abc";
  std::string g = "abcde";

  ASSERT_TRUE(comparability(r, e));
  ASSERT_TRUE(comparability(r, l));
  ASSERT_TRUE(comparability(r, g));

  ASSERT_EQ(r, e);
  ASSERT_LT(l, r);
  ASSERT_GT(r, l);
  ASSERT_LT(r, g);
  ASSERT_GT(g, r);
  ASSERT_LE(l, r);
  ASSERT_LE(r, g);
  ASSERT_GE(g, r);
  ASSERT_GE(r, l);
  ASSERT_LE(r, e);
  ASSERT_LE(e, r);
  ASSERT_GE(r, e);
  ASSERT_GE(e, r);
}

TEST(StringRef, cmpStdView) {
  StringRef r = "abcd";

  std::string_view e = "abcd";
  std::string_view l = "abc";
  std::string_view g = "abcde";

  ASSERT_TRUE(comparability(r, e));
  ASSERT_TRUE(comparability(r, l));
  ASSERT_TRUE(comparability(r, g));

  ASSERT_EQ(r, e);
  ASSERT_LT(l, r);
  ASSERT_GT(r, l);
  ASSERT_LT(r, g);
  ASSERT_GT(g, r);
  ASSERT_LE(l, r);
  ASSERT_LE(r, g);
  ASSERT_GE(g, r);
  ASSERT_GE(r, l);
  ASSERT_LE(r, e);
  ASSERT_LE(e, r);
  ASSERT_GE(r, e);
  ASSERT_GE(e, r);
}

TEST(StringRef, cmpCStr) {
  StringRef r = "abcd";

  char const* e = "abcd";
  char const* l = "abc";
  char const* g = "abcde";

  ASSERT_TRUE(comparability(r, e));
  ASSERT_TRUE(comparability(r, l));
  ASSERT_TRUE(comparability(r, g));

  ASSERT_EQ(r, e);
  ASSERT_LT(l, r);
  ASSERT_GT(r, l);
  ASSERT_LT(r, g);
  ASSERT_GT(g, r);
  ASSERT_LE(l, r);
  ASSERT_LE(r, g);
  ASSERT_GE(g, r);
  ASSERT_GE(r, l);
  ASSERT_LE(r, e);
  ASSERT_LE(e, r);
  ASSERT_GE(r, e);
  ASSERT_GE(e, r);
}

TEST(StringRef, cmpSH) {
  StringRef r = "abcd";

  StringRef srl = "abc";
  StringRef sre = "abcd";
  StringRef srg = "abcde";

  String sl = "abc";
  String se = "abcd";
  String sg = "abcde";

  StringView svl = "abc";
  StringView sve = "abcd";
  StringView svg = "abcde";

  std::string ssl = "abc";
  std::string sse = "abcd";
  std::string ssg = "abcde";

  std::string_view ssvl = "abc";
  std::string_view ssve = "abcd";
  std::string_view ssvg = "abcde";

  char const* cl = "abc";
  char const* ce = "abcd";
  char const* cg = "abcde";

  auto const lt = std::weak_ordering::less;
  auto const eq = std::weak_ordering::equivalent;
  auto const gt = std::weak_ordering::greater;

  ASSERT_EQ(r <=> srl, gt);
  ASSERT_EQ(r <=> sre, eq);
  ASSERT_EQ(r <=> srg, lt);

  ASSERT_EQ(r <=> sl, gt);
  ASSERT_EQ(r <=> se, eq);
  ASSERT_EQ(r <=> sg, lt);

  ASSERT_EQ(r <=> svl, gt);
  ASSERT_EQ(r <=> sve, eq);
  ASSERT_EQ(r <=> svg, lt);

  ASSERT_EQ(r <=> ssl, gt);
  ASSERT_EQ(r <=> sse, eq);
  ASSERT_EQ(r <=> ssg, lt);

  ASSERT_EQ(r <=> ssvl, gt);
  ASSERT_EQ(r <=> ssve, eq);
  ASSERT_EQ(r <=> ssvg, lt);

  ASSERT_EQ(r <=> cl, gt);
  ASSERT_EQ(r <=> ce, eq);
  ASSERT_EQ(r <=> cg, lt);

  ASSERT_EQ(r <=> "abc", gt);
  ASSERT_EQ(r <=> "abcd", eq);
  ASSERT_EQ(r <=> "abcde", lt);

  ASSERT_EQ(srl <=> r, lt);
  ASSERT_EQ(sre <=> r, eq);
  ASSERT_EQ(srg <=> r, gt);

  ASSERT_EQ(sl <=> r, lt);
  ASSERT_EQ(se <=> r, eq);
  ASSERT_EQ(sg <=> r, gt);

  ASSERT_EQ(svl <=> r, lt);
  ASSERT_EQ(sve <=> r, eq);
  ASSERT_EQ(svg <=> r, gt);

  ASSERT_EQ(ssl <=> r, lt);
  ASSERT_EQ(sse <=> r, eq);
  ASSERT_EQ(ssg <=> r, gt);

  ASSERT_EQ(ssvl <=> r, lt);
  ASSERT_EQ(ssve <=> r, eq);
  ASSERT_EQ(ssvg <=> r, gt);

  ASSERT_EQ(cl <=> r, lt);
  ASSERT_EQ(ce <=> r, eq);
  ASSERT_EQ(cg <=> r, gt);

  ASSERT_EQ("abc" <=> r, lt);
  ASSERT_EQ("abcd" <=> r, eq);
  ASSERT_EQ("abcde" <=> r, gt);
}
