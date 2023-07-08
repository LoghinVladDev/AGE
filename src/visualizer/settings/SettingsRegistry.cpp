//
// Created by Vlad-Andrei Loghin on 18.06.23.
//

#include "SettingsRegistry.hpp"
#include <CDS/filesystem/Path>
#include <CDS/threading/Thread>
#include <atomic>
#include <condition_variable>
#include <lang/filesystem/PathAwareFstream.hpp>
#include <mutex>
#include <platform/PathUtils.hpp>
#include <tuple>

namespace {
using namespace cds;
using namespace cds::filesystem;
using namespace cds::json;
using namespace age;
using namespace age::visualizer::settings;

using std::atomic_flag;
using std::condition_variable;
using std::lock_guard;
using std::mutex;
using std::tuple;
using std::unique_lock;

constexpr auto const paddingBufferSize = 128;
constexpr char const paddingBuffer[paddingBufferSize + 1] = "                                "
                                                            "                                "
                                                            "                                "
                                                            "                                ";

auto addIndent(auto& out, int indent) -> void {
  while (indent > 0) {
    out.write(paddingBuffer, std::min(indent, paddingBufferSize));
    indent -= paddingBufferSize;
  }
}

auto filteredDump(auto& out, JsonArray const& object, int currentIndent, int indent) -> void;
auto filteredDump(auto& out, JsonObject const& object, int currentIndent, int indent) -> void;

auto filteredDump(auto& out, JsonElement const& object, int currentIndent, int indent) -> void {
  if (object.isArray()) {
    filteredDump(out, object.getArray(), currentIndent, indent);
  }

  if (object.isString()) {
    out << '\"' << object.getString() << '\"';
  }

  if (object.isBoolean()) {
    out << (object.getBoolean() ? "true" : "false");
  }

  if (object.isLong()) {
    out << object.getLong();
  }

  if (object.isDouble()) {
    out << std::showpoint << object.getDouble();
  }
}

auto filteredDump(auto& out, JsonArray const& object, int currentIndent, int indent) -> void {
  if (object.empty()) {
    out << "[]";
    return;
  }

  out << "[\n";
  auto const nextIndent = currentIndent + indent;

  auto it = object.begin();
  addIndent(out, nextIndent);
  filteredDump(out, *it, nextIndent, indent);
  ++it;

  for (auto end = object.end(); it != end; ++it) {
    if (it->isJson()) {
      continue;
    }

    out << ",\n";

    addIndent(out, nextIndent);
    filteredDump(out, *it, nextIndent, indent);
  }

  out << "]";
}

auto filteredDump(auto& out, JsonObject const& object, int currentIndent, int indent) -> void {
  if (object.empty()) {
    out << "{}";
    return;
  }

  out << "{\n";
  auto const nextIndent = currentIndent + indent;

  auto it = object.begin();
  addIndent(out, nextIndent);
  out << '\"' << it->key() << "\" : ";
  filteredDump(out, it->value(), nextIndent, indent);
  ++it;

  for (auto end = object.end(); it != end; ++it) {
    if (it->value().isJson()) {
      continue;
    }

    out << ",\n";

    addIndent(out, nextIndent);
    out << '\"' << it->key() << "\" : ";
    filteredDump(out, it->value(), nextIndent, indent);
  }

  out << "\n}";
}

auto filteredDump(auto& out, JsonObject const& object, int indent = 2) -> void { filteredDump(out, object, 0, indent); }

auto sub(StringRef& key) noexcept -> StringRef {
  auto dotPos = key.find('.');
  if (dotPos == StringRef::npos) {
    return cds::exchange(key, "");
  }

  auto rVal = key.sub(0, dotPos);
  key = key.dropFront(dotPos + 1);
  return rVal;
}

auto get(auto& json, StringRef key) noexcept(false) -> auto& {
  auto current = &json;
  auto subKey = sub(key);
  while (key) {
    current = &current->getJson(subKey);
    subKey = sub(key);
  }

  return current->get(subKey);
}

auto convertToPath(StringRef key) noexcept -> String {
  String path = Registry::defaultPath;
  path += directorySeparator;
  while (key) {
    auto dotPos = key.find('.');
    if (dotPos == StringRef::npos) {
      dotPos = key.size();
    }

    path += StringView(key.takeFront(dotPos));
    key = key.dropFront(dotPos + 1);
    if (key) {
      path += directorySeparator;
    }
  }
  return path + ".json";
}

UniquePointer<Registry> _registry = nullptr;
} // namespace

auto loaderFn(JsonObject* main, JsonObject* copy) {
  main->put("testStr", "test");
  main->put("testInt", 0);
  main->put("testFloat", 0.0f);
  main->put("testBool", false);
  main->put("testArray", JsonArray());
  main->put("testJson", JsonObject().put("testStr", "testSub"));
  *copy = *main;
}

auto saveUnderlying(Path const& path, String const& key, JsonObject const& json) -> void {
  std::cout << "  Indirectly saving: " << path / (key + ".json") << std::endl;

  for (auto const& entry : json) {
    if (entry.value().isJson()) {
      saveUnderlying(path / key, entry.key(), entry.value().getJson());
    }
  }

  PathAwareOfstream outFile((path / (key + ".json")).toString());
  filteredDump(outFile, json);
}

auto saverFn(Path const& path, JsonObject const* json) {
  std::cout << "Started save of: " << path.toString() << std::endl;

  for (auto const& entry : *json) {
    if (entry.value().isJson()) {
      saveUnderlying(path.parent(), entry.key(), entry.value().getJson());
    }
  }

  PathAwareOfstream outFile(path.toString());
  filteredDump(outFile, *json);
}

auto Registry::sub(StringRef& key) noexcept -> StringRef { return ::sub(key); }

auto Registry::active() noexcept(false) -> Registry& {
  if (!_registry) {
    _registry = cds::makeUnique<Registry>(Token {});
  }

  if (!_registry->_loaded) {
    _registry->_loader->await();
    _registry->_loaded = true;
  }

  return *_registry;
}

Registry::Registry([[maybe_unused]] Token) noexcept :
    _loader(cds::makeUnique<AsyncRunner<void, JsonObject*, JsonObject*>>(loaderFn)),
    _saver(cds::makeUnique<AsyncRunner<void, Path, JsonObject const*>>(saverFn)) {
  _loader->trigger(&_active, &_stored);
}

auto Registry::reset(StringRef key) noexcept(false) -> void {
  auto* lJson = &_active;
  auto* rJson = &_stored;

  if (!key) {
    *lJson = *rJson;
    return;
  }

  auto subKey = sub(key);
  while (key) {
    lJson = &lJson->getJson(subKey);
    rJson = &rJson->getJson(subKey);
    subKey = sub(key);
  }

  lJson->get(subKey) = rJson->get(subKey);
}

auto Registry::replaceIfMissing(JsonObject* pJson, StringRef key, bool overwriteType) noexcept -> void {
  if (auto jsonIt = pJson->find(key); jsonIt == pJson->end()) {
    pJson->put(key, JsonObject());
  } else if (overwriteType || !jsonIt->value().isJson()) {
    jsonIt->value() = JsonObject();
  }
}

auto Registry::save(StringRef key) noexcept(false) -> void {
  _saver->await();
  auto lJson = &_stored;
  auto rJson = &_active;
  String savePath = key ? convertToPath(key) : rootFileName;

  if (key) {
    auto subKey = sub(key);
    while (key) {
      replaceIfMissing(lJson, subKey, true);

      lJson = &lJson->getJson(subKey);
      rJson = &rJson->getJson(subKey);
      subKey = sub(key);
    }
    replaceIfMissing(lJson, subKey);
    auto& lSub = lJson->get(subKey);
    auto const& rSub = rJson->get(subKey);
    lSub = rSub;
    if (lSub.isJson()) {
      lJson = &lSub.getJson();
    }
  } else {
    *lJson = *rJson;
  }

  _saver->trigger(savePath, lJson);
}

auto Registry::getInt(StringRef key) const noexcept(false) -> int { return get(_active, key).getInt(); }
auto Registry::getLong(StringRef key) const noexcept(false) -> long { return get(_active, key).getLong(); }
auto Registry::getFloat(StringRef key) const noexcept(false) -> float { return get(_active, key).getFloat(); }
auto Registry::getDouble(StringRef key) const noexcept(false) -> double { return get(_active, key).getDouble(); }
auto Registry::getString(StringRef key) const noexcept(false) -> String const& { return get(_active, key).getString(); }
auto Registry::getJson(StringRef key) const noexcept(false) -> JsonObject const& { return get(_active, key).getJson(); }
auto Registry::getString(StringRef key) noexcept(false) -> String& { return get(_active, key).getString(); }
auto Registry::getArray(StringRef key) noexcept(false) -> JsonArray& { return get(_active, key).getArray(); }
auto Registry::getJson(StringRef key) noexcept(false) -> JsonObject& { return get(_active, key).getJson(); }

auto Registry::getArray(StringRef key) const noexcept(false) -> JsonArray const& {
  return get(_active, key).getArray();
}

Registry::~Registry() noexcept {
  std::cout << dump(registry()._stored) << '\n';
  _saver->await();
  _loader->await();
}

auto Registry::awaitPending() noexcept -> void {
  auto& r = registry();
  r._saver->await();
  r._loader->await();
}
