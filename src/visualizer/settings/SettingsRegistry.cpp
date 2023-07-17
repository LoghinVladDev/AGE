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

auto filteredDump(ArrayRef<StringRef>& integrals, auto& out, JsonArray const& object, int currentIndent, int indent) -> void;
auto filteredDump(ArrayRef<StringRef>& integrals, auto& out, JsonObject const& object, int currentIndent, int indent) -> void;

auto filteredDump(ArrayRef<StringRef>& integrals, auto& out, JsonElement const& object, int currentIndent, int indent) -> void {
  if (object.isJson()) {
    filteredDump(integrals, out, object.getJson(), currentIndent, indent);
  }

  if (object.isArray()) {
    filteredDump(integrals, out, object.getArray(), currentIndent, indent);
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

auto filteredDump(ArrayRef<StringRef>& integrals, auto& out, JsonArray const& object, int currentIndent, int indent) -> void {
  if (object.empty()) {
    out << "[]";
    return;
  }

  out << "[\n";
  auto const nextIndent = currentIndent + indent;

  auto it = object.begin();
  addIndent(out, nextIndent);
  filteredDump(integrals, out, *it, nextIndent, indent);
  ++it;

  for (auto end = object.end(); it != end; ++it) {
    out << ",\n";
    addIndent(out, nextIndent);
    filteredDump(integrals, out, *it, nextIndent, indent);
  }

  out << "\n";
  addIndent(out, currentIndent);
  out << "]";
}

auto filteredDump(ArrayRef<StringRef>& integrals, auto& out, JsonObject const& object, int currentIndent, int indent) -> void {
  if (object.empty() || object.count([](auto const& e) { return !e.value().isJson(); }) == 0u) {
    out << "{}";
    return;
  }

  out << "{\n";
  auto const nextIndent = currentIndent + indent;

  auto it = object.begin();
  bool skippedFirst = true;
  if (!it->value().isJson()) {
    skippedFirst = false;
    addIndent(out, nextIndent);
    out << '\"' << it->key() << "\" : ";
    filteredDump(integrals, out, it->value(), nextIndent, indent);
  }
  ++it;

  for (auto end = object.end(); it != end; ++it) {
    if (it->value().isJson()) {
      continue;
    }

    if (!skippedFirst) {
      out << ",\n";
      skippedFirst = false;
    }

    addIndent(out, nextIndent);
    out << '\"' << it->key() << "\" : ";
    filteredDump(integrals, out, it->value(), nextIndent, indent);
  }

  out << "\n";
  addIndent(out, currentIndent);
  out << "}";
}

auto filteredDump(ArrayRef<StringRef>& integrals, auto& out, JsonObject const& object, int indent = 2) -> void {
  filteredDump(integrals, out, object, 0, indent);
}

auto sub(StringRef& key) noexcept -> StringRef {
  auto dotPos = key.find('.');
  if (dotPos == StringRef::npos) {
    return cds::exchange(key, "");
  }

  auto rVal = key.sub(0, dotPos);
  key = key.dropFront(dotPos + 1);
  return rVal;
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

auto recursiveLoad(Map<String, JsonElement>& map, Path const& path) noexcept -> void {
  for (auto const& entry : path.walk(0u)) {
    for (auto const& file : entry.files()) {
      if (!file.endsWith(".json")) {
        continue;
      }

      if ((path / file).toString() == Registry::rootFileName) {
        continue;
      }

      StringView key {file.cStr(), file.length() - 5u};
      try {
        map.emplace(key, loadJson(path / file));
      } catch (cds::Exception const& unexpectedError) {
        std::cerr << "Invalid error while initialising settings group '" << key << "': " << unexpectedError
                  << ". Settings will return to default" << std::endl;
      } catch (std::exception const&) {
        std::cerr << "Failed to open file for settings group '" << key << "'. Settings will return to default"
                  << std::endl;
      }
    }

    for (auto const& directory : entry.directories()) {
      try {
        recursiveLoad(map.emplace(directory, JsonObject()).value().getJson(), path / directory);
      } catch (cds::Exception const& typeException) {
        std::cerr << "Settings group directory found for key '" << directory
                  << "', but already in use in primary json by a different data-type: " << typeException
                  << ". This will not be overwritten." << std::endl;
      }
    }
  }
}

auto loaderFn(JsonObject* main, JsonObject* copy) noexcept {
  auto configPath = Path(Registry::defaultPath);
  try {
    *main = loadJson(Registry::rootFileName);
    recursiveLoad(*main, configPath);
  } catch (cds::Exception const& unexpectedError) {
    std::cerr << "Invalid error while initialising settings: " << unexpectedError << ". Settings will return to default"
              << std::endl;
  } catch (std::exception const&) {
    std::cerr << "Root config not found. Settings will return to default" << std::endl;
  }

  *copy = *main;
  std::cout << "Loaded config:\n" << dump(*main, 2u) << "\n";
}

auto saveUnderlying(ArrayRef<StringRef>& filtered, Path const& path, String const& key, JsonObject const& json) -> void {
  std::cout << "Saved underlying config json '" << path.toString() + "/" + key + ".json"
            << "':\n"
            << dump(json, 2u) << "\n";
  for (auto const& entry : json) {
    if (entry.value().isJson()) {
      saveUnderlying(filtered, path / key, entry.key(), entry.value().getJson());
    }
  }

  PathAwareOfstream outFile((path / (key + ".json")).toString());
  filteredDump(filtered, outFile, json);
}

auto saverFn(ArrayRef<StringRef> filtered, Path const& path, JsonObject const* json) {
  auto targetPath = path.parent() / path.node().toString().removeSuffix(".json");
  if (path.toString() == Registry::rootFileName) {
    targetPath = path.parent();
  }

  std::cout << "Saved config json '" << targetPath.toString() + ".json"
            << "':\n"
            << dump(*json, 2u) << "\n";
  for (auto const& entry : *json) {
    if (entry.value().isJson()) {
      saveUnderlying(filtered, targetPath, entry.key(), entry.value().getJson());
    }
  }

  PathAwareOfstream outFile(path.toString());
  filteredDump(filtered, outFile, *json);
}
} // namespace

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
    _saver(cds::makeUnique<AsyncRunner<void, ArrayRef<StringRef>, Path, JsonObject const*>>(saverFn)) {
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

auto Registry::replaceIfMissing(JsonObject* pJson, StringRef key, bool overwriteType) noexcept -> bool {
  if (auto jsonIt = pJson->find(key); jsonIt == pJson->end()) {
    pJson->put(key, JsonObject());
    return true;
  } else if (overwriteType || !jsonIt->value().isJson()) {
    auto overwritten = !jsonIt->value().isJson();
    jsonIt->value() = JsonObject();
    return overwritten;
  }
  return false;
}

auto Registry::save(StringRef key) noexcept(false) -> void {
  _saver->await();
  auto lJson = &_stored;
  auto rJson = &_active;
  auto overwrittenSaveRoot = false;
  String savePath = key ? convertToPath(key) : rootFileName;

  if (key) {
    auto subKey = sub(key);
    while (key) {
      if (replaceIfMissing(lJson, subKey, true)) {
        savePath = convertToPath(key);
        break;
      }

      lJson = &lJson->getJson(subKey);
      rJson = &rJson->getJson(subKey);
      subKey = sub(key);
    }

    overwrittenSaveRoot = replaceIfMissing(lJson, subKey);
    if (!overwrittenSaveRoot) {
      auto& lSub = lJson->get(subKey);
      auto const& rSub = rJson->get(subKey);
      bool saveUnderlying = false;
      if (lSub.isJson()) {
        saveUnderlying = true;
      }

      lSub = rSub;
      if (saveUnderlying) {
        lJson = &lSub.getJson();
      }
    } else {
      savePath = rootFileName;
      *lJson = *rJson;
    }
  } else {
    *lJson = *rJson;
  }

  Array<StringRef> integralPathsRefs {_integralPaths.begin(), _integralPaths.end()};
  _saver->trigger(integralPathsRefs, savePath, lJson);
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
  _saver->await();
  _loader->await();
}

auto Registry::awaitPending() noexcept -> void {
  auto& r = registry();
  r._saver->await();
  r._loader->await();
}

auto Registry::getIntOr(StringRef key, int value) noexcept(false) -> int {
  // TODO: This can be improved to partially search and then fill
  try {
    return get(_active, key).getInt();
  } catch (cds::KeyException const&) {
    put(key, value);
    return get(_active, key).getInt();
  }
}

auto Registry::getLongOr(StringRef key, long value) noexcept(false) -> long {
  // TODO: This can be improved to partially search and then fill
  try {
    return get(_active, key).getLong();
  } catch (cds::KeyException const&) {
    put(key, static_cast<long long>(value));
    return get(_active, key).getLong();
  }
}

auto Registry::getFloatOr(StringRef key, float value) noexcept(false) -> float {
  // TODO: This can be improved to partially search and then fill
  try {
    return get(_active, key).getFloat();
  } catch (cds::KeyException const&) {
    put(key, value);
    return get(_active, key).getFloat();
  }
}

auto Registry::getDoubleOr(StringRef key, double value) noexcept(false) -> double {
  // TODO: This can be improved to partially search and then fill
  try {
    return get(_active, key).getDouble();
  } catch (cds::KeyException const&) {
    put(key, value);
    return get(_active, key).getDouble();
  }
}

auto Registry::getBooleanOr(StringRef key, bool value) noexcept(false) -> bool {
  // TODO: This can be improved to partially search and then fill
  try {
    return get(_active, key).getBoolean();
  } catch (cds::KeyException const&) {
    put(key, value);
    return get(_active, key).getBoolean();
  }
}

auto Registry::markAsIntegral(StringRef key) noexcept(false) -> void {
  _integralPaths.emplaceBack(key);
}

auto Registry::markAsComposite(StringRef key) noexcept(false) -> void {
  _integralPaths.removeAllThat([&key](auto const& path) { return path == key; });
}

auto Registry::isIntegral(StringRef key) const noexcept(false) -> bool {
  return _integralPaths.findFirstThat([&key](auto const& path){ return key == path; }) != _integralPaths.end();
}

auto Registry::isComposite(StringRef key) const noexcept(false) -> bool {
  return !isIntegral(key);
}
