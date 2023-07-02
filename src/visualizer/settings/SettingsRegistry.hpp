//
// Created by Vlad-Andrei Loghin on 18.06.23.
//

#ifndef AGE_SETTINGS_REGISTRY_HPP
#define AGE_SETTINGS_REGISTRY_HPP

#include <CDS/Union>
#include <CDS/memory/UniquePointer>
#include <CDS/util/JSON>

#include <lang/string/StringRef.hpp>

namespace age::visualizer::settings {
class Registry {
public:
  static auto active() noexcept(false) -> Registry&;
  static auto triggerLoad() noexcept(false) -> void;
  static auto reset(StringRef key = "") noexcept(false) -> void;
  static auto save(StringRef key = "") noexcept(false) -> void;

private:
  class Token {
    friend auto ::age::visualizer::settings::Registry::triggerLoad() noexcept(false) -> void;
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

  explicit(false) Registry(Token) noexcept {}

private:
  static auto sub(StringRef& key) noexcept -> StringRef;

  cds::json::JsonObject _contents;

  static inline cds::UniquePointer<Registry> _active;
  static inline cds::UniquePointer<Registry> _stored;
  static constexpr auto const defaultPath = "./registryBase.json";
  static constexpr cds::StringView const pathInternalPrefix = "__resourcepath__";
};

template <typename Type> auto Registry::put(StringRef key, Type&& value) noexcept(false) -> Registry& {
  auto current = &_contents;
  auto subKey = sub(key);
  while (key) {
    current = &current->getJson(subKey);
    subKey = sub(key);
  }
  current->put(subKey, std::forward<Type>(value));
  return *this;
}
} // namespace age::visualizer::settings

#endif // AGE_SETTINGS_REGISTRY_HPP
