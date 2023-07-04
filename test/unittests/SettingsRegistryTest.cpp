//
// Created by Vlad-Andrei Loghin on 02.07.23.
//

#include <gtest/gtest.h>
#include <visualizer/settings/SettingsRegistry.hpp>

namespace {
using namespace age::visualizer::settings;
using namespace cds::json;
} // namespace

TEST(SettingsRegistryTest, preemptiveLoad) {
  auto& r = Registry::active();
  auto const& cr = Registry::active();
  ASSERT_EQ(r.getString("testStr"), "test");
  ASSERT_EQ(r.getInt("testInt"), 0);
  ASSERT_EQ(r.getLong("testInt"), 0l);
  ASSERT_EQ(r.getFloat("testFloat"), 0.0f);
  ASSERT_EQ(r.getDouble("testFloat"), 0.0);
  ASSERT_TRUE(r.getArray("testArray").empty());
  ASSERT_EQ(r.getJson("testJson").size(), 1u);
  ASSERT_EQ(r.getJson("testJson").getString("testStr"), "testSub");
  ASSERT_TRUE(cr.getArray("testArray").empty());
  ASSERT_EQ(cr.getJson("testJson").size(), 1u);
  ASSERT_EQ(cr.getJson("testJson").getString("testStr"), "testSub");
  ASSERT_EQ(cr.getString("testStr"), "test");
  ASSERT_EQ(r.getString("testJson.testStr"), "testSub");
}

TEST(SettingsRegistryTest, put) {
  auto& r = Registry::active();
  r.put("put_testStr1", "test1");
  r.put("testJson.put_testStr1", "test2");

  ASSERT_EQ(r.getString("put_testStr1"), "test1");
  ASSERT_EQ(r.getString("testJson.put_testStr1"), "test2");
  ASSERT_EQ(r.getJson("testJson").getString("put_testStr1"), "test2");
}

TEST(SettingsRegistryTest, reset) {
  auto& r = Registry::active();
  r.put("reset_testStr1", "test1");
  r.put("testJson.reset_testStr1", "test2");

  r.getString("testJson.testStr") = "test3";
  ASSERT_EQ(r.getString("testJson.testStr"), "test3");
  Registry::reset("testJson.testStr");
  ASSERT_EQ(r.getString("testJson.testStr"), "testSub");

  Registry::reset("testJson");

  ASSERT_EQ(r.getString("reset_testStr1"), "test1");

  ASSERT_THROW((void) r.getString("testJson.reset_testStr1"), cds::KeyException);
  ASSERT_THROW((void) r.getJson("testJson").getString("reset_testStr1"), cds::KeyException);

  r.put("testJson.reset_testStr1", "test2");
  Registry::reset();
  ASSERT_THROW((void) r.getString("reset_testStr1"), cds::KeyException);
  ASSERT_THROW((void) r.getString("testJson.reset_testStr1"), cds::KeyException);
  ASSERT_THROW((void) r.getJson("testJson").getString("reset_testStr1"), cds::KeyException);
}

TEST(SettingsRegistryTest, save) {
  auto& r = Registry::active();

  r.put("save1_testStr1", "test1");
  r.getString("testStr") = "test2";
  r.put("testJson.save1_testStr2", "test3");
  r.getString("testJson.testStr") = "test4";
  Registry::save("testJson");
  Registry::reset();
  ASSERT_THROW((void) r.getString("save1_testStr1"), cds::KeyException);
  ASSERT_NE("testStr", "test2");
  ASSERT_EQ(r.getString("testJson.save1_testStr2"), "test3");
  ASSERT_EQ(r.getString("testJson.testStr"), "test4");

  r.put("save2_testStr1", "test1");
  r.getString("testStr") = "test2";
  r.put("testJson.save2_testStr2", "test3");
  r.getString("testJson.testStr") = "test4";
  Registry::save();
  Registry::reset();
  ASSERT_EQ(r.getString("save2_testStr1"), "test1");
  ASSERT_EQ(r.getString("testStr"), "test2");
  ASSERT_EQ(r.getString("testJson.save2_testStr2"), "test3");
  ASSERT_EQ(r.getString("testJson.testStr"), "test4");
}

TEST(SettingsRegistryTest, savePath) {
  auto& r = Registry::active();

  r.put("save1_json", JsonObject());
  Registry::save("save1_json");
  Registry::reset();
  ASSERT_TRUE(r.getJson("save1_json").empty());

  r.put("save2_json.save3_json", JsonObject());
  Registry::save("save2_json.save3_json");
  Registry::reset();
  ASSERT_TRUE(r.getJson("save2_json.save3_json").empty());

  r.replace("save2_json.save3_json", "");
  Registry::save("save2_json.save3_json");
  Registry::reset();
  ASSERT_TRUE(r.getString("save2_json.save3_json").empty());
  r.replace("save2_json.save3_json", JsonObject());
  Registry::save("save2_json.save3_json");
  Registry::reset();
  ASSERT_TRUE(r.getJson("save2_json.save3_json").empty());

  r.replace("save2_json.save3_json", "");
  Registry::save("save2_json.save3_json");
  Registry::reset();
  ASSERT_TRUE(r.getString("save2_json.save3_json").empty());
  r.replace("save2_json.save3_json.save4_json", JsonObject());
  Registry::save("save2_json.save3_json.save4_json");
  Registry::reset();
  ASSERT_TRUE(r.getJson("save2_json.save3_json.save4_json").empty());
}