//
// Created by Vlad-Andrei Loghin on 18.06.23.
//

#pragma once

#include <CDS/Union>
#include <CDS/memory/UniquePointer>
#include <CDS/util/JSON>

#include <lang/array/ArrayRef.hpp>
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

  [[nodiscard]] auto getIntOr(StringRef key, int value) noexcept(false) -> int;
  [[nodiscard]] auto getLongOr(StringRef key, long value) noexcept(false) -> long;
  [[nodiscard]] auto getFloatOr(StringRef key, float value) noexcept(false) -> float;
  [[nodiscard]] auto getDoubleOr(StringRef key, double value) noexcept(false) -> double;
  [[nodiscard]] auto getBooleanOr(StringRef key, bool value) noexcept(false) -> bool;

  auto markAsIntegral(StringRef key) noexcept(false) -> void;
  auto markAsComposite(StringRef key) noexcept(false) -> void;
  auto isIntegral(StringRef key) const noexcept(false) -> bool;
  auto isComposite(StringRef key) const noexcept(false) -> bool;

  template <typename Convertible> [[nodiscard]] auto getStringOr(StringRef key, Convertible&& value) noexcept(false)
      -> cds::String const&;
  template <typename Convertible> [[nodiscard]] auto getArrayOr(StringRef key, Convertible&& value) noexcept(false)
      -> cds::json::JsonArray const&;
  template <typename Convertible> [[nodiscard]] auto getJsonOr(StringRef key, Convertible&& value) noexcept(false)
      -> cds::json::JsonObject const&;

  template <typename Type> auto put(StringRef key, Type&& value) noexcept(false) -> Registry&;
  template <typename Type> auto replace(StringRef key, Type&& value) noexcept(false) -> Registry&;

  explicit(false) Registry(Token) noexcept;
  ~Registry() noexcept;

  static constexpr auto const defaultPath = "./config";
  static constexpr auto const rootFileName = "./config/registryBase.json";

private:
  static auto sub(StringRef& key) noexcept -> StringRef;
  static auto replaceIfMissing(cds::json::JsonObject* pJson, StringRef key, bool overwriteType = false) noexcept
      -> bool;
  static auto get(auto& json, StringRef key) noexcept(false) -> auto&;

  bool _loaded = false;
  cds::json::JsonObject _active;
  cds::json::JsonObject _stored;
  cds::UniquePointer<AsyncRunner<void, cds::json::JsonObject*, cds::json::JsonObject*>> const _loader;
  cds::UniquePointer<AsyncRunner<void, ArrayRef<StringRef>, cds::filesystem::Path, cds::json::JsonObject const*>> const _saver;
  cds::Array<cds::String> _integralPaths;
  static constexpr cds::StringView const pathInternalPrefix = "__resourcepath__";
  static inline cds::UniquePointer<Registry> _registry = nullptr;
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

inline auto Registry::get(auto& json, StringRef key) noexcept(false) -> auto& {
  auto current = &json;
  auto subKey = sub(key);
  while (key) {
    current = &current->getJson(subKey);
    subKey = sub(key);
  }

  return current->get(subKey);
}

template <typename Convertible> auto Registry::getStringOr(StringRef key, Convertible&& value) noexcept(false)
    -> cds::String const& {
  // TODO: This can be improved to partially search and then fill.
  try {
    return get(_active, key).getString();
  } catch (cds::KeyException const&) {
    put(key, std::forward<Convertible>(value));
    return get(_active, key).getString();
  }
}

template <typename Convertible> auto Registry::getArrayOr(StringRef key, Convertible&& value) noexcept(false)
    -> cds::json::JsonArray const& {
  // TODO: This can be improved to partially search and then fill.
  try {
    return get(_active, key).getArray();
  } catch (cds::KeyException const&) {
    put(key, std::forward<Convertible>(value));
    return get(_active, key).getArray();
  }
}

template <typename Convertible> auto Registry::getJsonOr(StringRef key, Convertible&& value) noexcept(false)
    -> cds::json::JsonObject const& {
  // TODO: This can be improved to partially search and then fill.
  try {
    return get(_active, key).getJson();
  } catch (cds::KeyException const&) {
    put(key, std::forward<Convertible>(value));
    return get(_active, key).getJson();
  }
}
} // namespace age::visualizer::settings
