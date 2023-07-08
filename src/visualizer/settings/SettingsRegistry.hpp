//
// Created by Vlad-Andrei Loghin on 18.06.23.
//

#ifndef AGE_SETTINGS_REGISTRY_HPP
#define AGE_SETTINGS_REGISTRY_HPP

#include <CDS/Union>
#include <CDS/memory/UniquePointer>
#include <CDS/util/JSON>

#include <lang/string/StringRef.hpp>
#include <lang/thread/AsyncRunner.hpp>

namespace age::visualizer::settings {
class Registry {
public:
  static inline auto triggerLoad() noexcept(false) -> void { (void) active(); }
  static auto awaitPending() noexcept -> void;
  static auto active() noexcept(false) -> Registry&;
  auto reset(StringRef key = "") noexcept(false) -> void;
  auto save(StringRef key = "") noexcept(false) -> void;

private:
  class Token {
    friend auto ::age::visualizer::settings::Registry::active() noexcept(false) -> Registry&;
    Token() = default;
  };

public:
  [[nodiscard]] auto getInt(StringRef key) const noexcept(false) -> int;
  [[nodiscard]] auto getLong(StringRef key) const noexcept(false) -> long;
  [[nodiscard]] auto getFloat(StringRef key) const noexcept(false) -> float;
  [[nodiscard]] auto getDouble(StringRef key) const noexcept(false) -> double;
  [[nodiscard]] auto getString(StringRef key) const noexcept(false) -> cds::String const&;
  [[nodiscard]] auto getArray(StringRef key) const noexcept(false) -> cds::json::JsonArray const&;
  [[nodiscard]] auto getJson(StringRef key) const noexcept(false) -> cds::json::JsonObject const&;
  [[nodiscard]] auto getString(StringRef key) noexcept(false) -> cds::String&;
  [[nodiscard]] auto getArray(StringRef key) noexcept(false) -> cds::json::JsonArray&;
  [[nodiscard]] auto getJson(StringRef key) noexcept(false) -> cds::json::JsonObject&;

  template <typename Type> auto put(StringRef key, Type&& value) noexcept(false) -> Registry&;
  template <typename Type> auto replace(StringRef key, Type&& value) noexcept(false) -> Registry&;

  explicit(false) Registry(Token) noexcept;
  ~Registry() noexcept;

  static constexpr auto const defaultPath = "./config";
  static constexpr auto const rootFileName = "./config/registryBase.json";

private:
  static auto sub(StringRef& key) noexcept -> StringRef;
  static auto replaceIfMissing(cds::json::JsonObject* pJson, StringRef key, bool overwriteType = false) noexcept
      -> void;

  bool _loaded = false;
  cds::json::JsonObject _active;
  cds::json::JsonObject _stored;
  cds::UniquePointer<AsyncRunner<void, cds::json::JsonObject*, cds::json::JsonObject*>> const _loader;
  cds::UniquePointer<AsyncRunner<void, cds::filesystem::Path, cds::json::JsonObject const*>> const _saver;
  static constexpr cds::StringView const pathInternalPrefix = "__resourcepath__";
};

inline auto registry() noexcept(false) -> Registry& { return Registry::active(); }

template <typename Type> auto Registry::put(StringRef key, Type&& value) noexcept(false) -> Registry& {
  auto current = &_active;
  auto subKey = sub(key);
  while (key) {
    replaceIfMissing(current, subKey);
    current = &current->getJson(subKey);
    subKey = sub(key);
  }
  current->put(subKey, std::forward<Type>(value));
  return *this;
}

template <typename Type> auto Registry::replace(StringRef key, Type&& value) noexcept(false) -> Registry& {
  auto current = &_active;
  auto subKey = sub(key);
  while (key) {
    replaceIfMissing(current, subKey);
    current = &current->getJson(subKey);
    subKey = sub(key);
  }

  if (auto it = current->find(subKey); it != current->end()) {
    it->value() = std::forward<Type>(value);
  } else {
    current->put(subKey, std::forward<Type>(value));
  }

  return *this;
}
} // namespace age::visualizer::settings

#endif // AGE_SETTINGS_REGISTRY_HPP
