//
// Created by Vlad-Andrei Loghin on 02.07.23.
//

#include <filesystem>
#include <gtest/gtest.h>
#include <visualizer/settings/SettingsRegistry.hpp>

#include <CDS/filesystem/Path>

namespace {
using age::visualizer::settings::registry;
using namespace cds::json;
} // namespace

TEST(SettingsRegistryTest, storeExistingConfig) {
  std::filesystem::remove_all("./.config_backup");
  if (std::filesystem::exists("./config")) {
    std::filesystem::copy("./config", "./.config_backup",
                          std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
    std::filesystem::remove_all("./config");
  }
}

TEST(SettingsRegistryTest, prepareTestConfig) {
  std::filesystem::create_directory("./config");
  std::ofstream registryBase("./config/registryBase.json");
  registryBase <<
      R"({
  "testStr" : "test",
  "testInt" : 0,
  "testFloat" : 0.0,
  "testBool" : false,
  "testArray" : []
})";

  std::ofstream testJson("./config/testJson.json");
  testJson <<
      R"({
  "testStr" : "testSub"
})";

  std::ofstream notAJson("./config/testXml.xml");
  notAJson <<
      R"(<test>
  <property>test</property>
</test>
)";

  std::ofstream erronousJson1("./config/errJson1.json");
  erronousJson1 << R"({)";

  std::ofstream erronousJson2("./config/errJson2.json");
  erronousJson2 << R"({ "tes })";

  std::ofstream erronousJson3("./config/errJson3.json");
  erronousJson3 << R"({ "tes" : "te })";

  std::ofstream readProtFile("./config/readProt.json");
  readProtFile << "{}\n";
  readProtFile.close();
  std::filesystem::permissions("./config/readProt.json",
                               std::filesystem::perms::group_read | std::filesystem::perms::others_read
                                   | std::filesystem::perms::owner_read,
                               std::filesystem::perm_options::remove);

  std::filesystem::create_directories("./config/badly_referencing");
  std::filesystem::create_directories("./config/badly_referencing/bad_key_not_json");
  std::ofstream badlyReferenced("./config/badly_referencing/bad_key_not_json.json");
  std::ofstream referencing("./config/badly_referencing.json");

  badlyReferenced << "{}";
  referencing <<
      R"({
  "bad_key_not_json" : "test"
})";
}

TEST(SettingsRegistryTest, preemptiveLoad) {
  auto& r = registry();
  auto const& cr = registry();
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
  auto& r = registry();
  r.put("put_testStr1", "test1");
  r.put("testJson.put_testStr1", "test2");

  ASSERT_EQ(r.getString("put_testStr1"), "test1");
  ASSERT_EQ(r.getString("testJson.put_testStr1"), "test2");
  ASSERT_EQ(r.getJson("testJson").getString("put_testStr1"), "test2");
}

TEST(SettingsRegistryTest, reset) {
  auto& r = registry();
  r.put("reset_testStr1", "test1");
  r.put("testJson.reset_testStr1", "test2");

  r.getString("testJson.testStr") = "test3";
  ASSERT_EQ(r.getString("testJson.testStr"), "test3");
  r.reset("testJson.testStr");
  ASSERT_EQ(r.getString("testJson.testStr"), "testSub");

  r.reset("testJson");

  ASSERT_EQ(r.getString("reset_testStr1"), "test1");

  ASSERT_THROW((void) r.getString("testJson.reset_testStr1"), cds::KeyException);
  ASSERT_THROW((void) r.getJson("testJson").getString("reset_testStr1"), cds::KeyException);

  r.put("testJson.reset_testStr1", "test2");
  r.reset();
  ASSERT_THROW((void) r.getString("reset_testStr1"), cds::KeyException);
  ASSERT_THROW((void) r.getString("testJson.reset_testStr1"), cds::KeyException);
  ASSERT_THROW((void) r.getJson("testJson").getString("reset_testStr1"), cds::KeyException);
}

TEST(SettingsRegistryTest, save) {
  auto& r = registry();

  r.put("save1_testStr1", "test1");
  r.getString("testStr") = "test2";
  r.put("testJson.save1_testStr2", "test3");
  r.getString("testJson.testStr") = "test4";
  r.save("testJson");
  r.reset();
  ASSERT_THROW((void) r.getString("save1_testStr1"), cds::KeyException);
  ASSERT_NE("testStr", "test2");
  ASSERT_EQ(r.getString("testJson.save1_testStr2"), "test3");
  ASSERT_EQ(r.getString("testJson.testStr"), "test4");

  r.put("save2_testStr1", "test1");
  r.getString("testStr") = "test2";
  r.put("testJson.save2_testStr2", "test3");
  r.getString("testJson.testStr") = "test4";
  r.save();
  r.reset();
  ASSERT_EQ(r.getString("save2_testStr1"), "test1");
  ASSERT_EQ(r.getString("testStr"), "test2");
  ASSERT_EQ(r.getString("testJson.save2_testStr2"), "test3");
  ASSERT_EQ(r.getString("testJson.testStr"), "test4");
}

TEST(SettingsRegistryTest, savePath) {
  auto& r = registry();

  r.put("save1_json", JsonObject());
  r.save("save1_json");
  r.reset();
  ASSERT_TRUE(r.getJson("save1_json").empty());

  r.put("save2_json.save3_json", JsonObject());
  r.save("save2_json.save3_json");
  r.reset();
  ASSERT_TRUE(r.getJson("save2_json.save3_json").empty());

  r.replace("save2_json.save3_json", "");
  r.save("save2_json.save3_json");
  r.reset();
  ASSERT_TRUE(r.getString("save2_json.save3_json").empty());
  r.replace("save2_json.save3_json", JsonObject());
  r.save("save2_json.save3_json");
  r.reset();
  ASSERT_TRUE(r.getJson("save2_json.save3_json").empty());

  r.replace("save2_json.save3_json", "");
  r.save("save2_json.save3_json");
  r.reset();
  ASSERT_TRUE(r.getString("save2_json.save3_json").empty());
  r.replace("save2_json.save3_json.save4_json", JsonObject());
  r.save("save2_json.save3_json.save4_json");
  r.reset();
  ASSERT_TRUE(r.getJson("save2_json.save3_json.save4_json").empty());
}

TEST(SettingsRegistryTest, extrasForCoverage) {
  auto& r = registry();
  r.put("testArray_filtered", JsonArray().pushBack(JsonObject()).pushBack("testStr123"));
  r.put("testArray_filtered2", JsonArray().pushBack("testStr1"));
  r.put("testArray_filtered3", JsonArray().pushBack("testStr1").pushBack(JsonObject()));
  r.put("testArray_filtered4", JsonArray().pushBack("testStr1").pushBack(JsonObject()).pushBack("testStr2"));
  r.put("testBoolTrue", true);

  (void) r.getIntOr("testIntOr", 2);
  (void) r.getLongOr("testLongOr", 4l);
  (void) r.getFloatOr("testFloatOr", 0.4f);
  (void) r.getDoubleOr("testDoubleOr", 0.8);
  (void) r.getBooleanOr("testBoolOr", false);
  (void) r.getStringOr("testStrOr", "testEmpty");
  (void) r.getArrayOr("testArrayOr", JsonArray().pushBack(3).pushBack(0.4));
  (void) r.getJsonOr("testJsonOr", JsonObject().put("test", "test").put("test2", 2));
  r.save();
}

TEST(SettingsRegistryTest, checkSavedConfigs) {
  age::visualizer::settings::Registry::awaitPending();
  ASSERT_TRUE(std::filesystem::exists("./config"));
  ASSERT_TRUE(std::filesystem::exists("./config/save2_json"));
  ASSERT_TRUE(std::filesystem::exists("./config/save2_json/save3_json"));
  ASSERT_TRUE(std::filesystem::exists("./config/save2_json/save3_json"));

  ASSERT_TRUE(std::filesystem::is_directory("./config"));
  ASSERT_TRUE(std::filesystem::is_directory("./config/save2_json"));
  ASSERT_TRUE(std::filesystem::is_directory("./config/save2_json/save3_json"));

  ASSERT_TRUE(std::filesystem::exists("./config/registryBase.json"));
  ASSERT_TRUE(std::filesystem::exists("./config/save1_json.json"));
  ASSERT_TRUE(std::filesystem::exists("./config/testJson.json"));

  ASSERT_TRUE(std::filesystem::is_regular_file("./config/registryBase.json"));
  ASSERT_TRUE(std::filesystem::is_regular_file("./config/save1_json.json"));
  ASSERT_TRUE(std::filesystem::is_regular_file("./config/testJson.json"));

  ASSERT_TRUE(std::filesystem::exists("./config/save2_json/save3_json.json"));
  ASSERT_TRUE(std::filesystem::is_regular_file("./config/save2_json/save3_json.json"));

  ASSERT_TRUE(std::filesystem::exists("./config/save2_json/save3_json/save4_json.json"));
  ASSERT_TRUE(std::filesystem::is_regular_file("./config/save2_json/save3_json/save4_json.json"));

  ASSERT_TRUE(std::filesystem::exists("./config/save2_json.json"));
  ASSERT_TRUE(std::filesystem::is_regular_file("./config/save2_json.json"));

  auto registryBase = cds::json::loadJson("./config/registryBase.json");
  auto testJson = cds::json::loadJson("./config/testJson.json");
  auto save1_json = cds::json::loadJson("./config/save1_json.json");
  auto save2_json = cds::json::loadJson("./config/save2_json.json");
  auto save3_json = cds::json::loadJson("./config/save2_json/save3_json.json");
  auto save4_json = cds::json::loadJson("./config/save2_json/save3_json/save4_json.json");

  ASSERT_EQ(registryBase.keys().size(), 18u);
  ASSERT_TRUE(registryBase.keys().containsAllOf(
      {"testStr", "testInt", "testFloat", "testBool", "testArray", "save2_testStr1"}));
  ASSERT_EQ(registryBase.getString("testStr"), "test2");
  ASSERT_EQ(registryBase.getInt("testInt"), 0);
  ASSERT_EQ(registryBase.getFloat("testFloat"), 0.0f);
  ASSERT_EQ(registryBase.getBoolean("testBool"), false);
  ASSERT_TRUE(registryBase.getArray("testArray").empty());
  ASSERT_EQ(registryBase.getString("save2_testStr1"), "test1");

  ASSERT_EQ(registryBase.getArray("testArray_filtered").size(), 2u);
  ASSERT_TRUE(registryBase.getArray("testArray_filtered")[0].getJson().empty());
  ASSERT_EQ(registryBase.getArray("testArray_filtered")[1].getString(), "testStr123");

  ASSERT_EQ(registryBase.getArray("testArray_filtered2").size(), 1u);
  ASSERT_EQ(registryBase.getArray("testArray_filtered2")[0].getString(), "testStr1");

  ASSERT_EQ(registryBase.getArray("testArray_filtered3").size(), 2u);
  ASSERT_EQ(registryBase.getArray("testArray_filtered3")[0].getString(), "testStr1");
  ASSERT_TRUE(registryBase.getArray("testArray_filtered3")[1].getJson().empty());

  ASSERT_EQ(registryBase.getArray("testArray_filtered4").size(), 3u);
  ASSERT_EQ(registryBase.getArray("testArray_filtered4")[0].getString(), "testStr1");
  ASSERT_TRUE(registryBase.getArray("testArray_filtered4")[1].getJson().empty());
  ASSERT_EQ(registryBase.getArray("testArray_filtered4")[2].getString(), "testStr2");

  ASSERT_EQ(registryBase.getBoolean("testBoolTrue"), true);

  ASSERT_EQ(registryBase.getInt("testIntOr"), 2);
  ASSERT_EQ(registryBase.getLong("testLongOr"), 4l);
  ASSERT_EQ(registryBase.getFloat("testFloatOr"), 0.4f);
  ASSERT_EQ(registryBase.getDouble("testDoubleOr"), 0.8);
  ASSERT_EQ(registryBase.getBoolean("testBoolOr"), false);
  ASSERT_EQ(registryBase.getString("testStrOr"), "testEmpty");
  ASSERT_EQ(registryBase.getArray("testArrayOr").size(), 2u);
  ASSERT_EQ(registryBase.getArray("testArrayOr").getInt(0), 3);
  ASSERT_EQ(registryBase.getArray("testArrayOr").getFloat(1), 0.4f);

  ASSERT_TRUE(save1_json.empty());

  ASSERT_EQ(testJson.keys().size(), 3u);
  ASSERT_TRUE(testJson.keys().containsAllOf({"testStr", "save1_testStr2", "save2_testStr2"}));
  ASSERT_EQ(testJson.getString("testStr"), "test4");
  ASSERT_EQ(testJson.getString("save1_testStr2"), "test3");
  ASSERT_EQ(testJson.getString("save2_testStr2"), "test3");

  ASSERT_TRUE(save3_json.keys().empty());
  ASSERT_TRUE(save4_json.empty());
  ASSERT_TRUE(save2_json.empty());
}

TEST(SettingsRegistryTest, restoreBackup) {
  std::filesystem::remove_all("./config");
  if (std::filesystem::exists("./.config_backup")) {
    std::filesystem::copy("./.config_backup", "./config", std::filesystem::copy_options::recursive);
    std::filesystem::remove_all("./.config_backup");
  }
}
