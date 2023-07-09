//
// Created by Vlad-Andrei Loghin on 20.06.23.
//

#pragma once
#include <CDS/Object>
#include <string>

#include <lang/generic/ExplicitComparisonsFromSpaceship.hpp>
#include <lang/generic/OperatorsWithImplicitConstruction.hpp>

namespace age {
class StringRef :
    public ::age::meta::op::GenFromSpaceship<StringRef, std::weak_ordering>,
    public ::age::meta::op::AddWithImplicit<StringRef>,
    public ::age::meta::op::SpaceshipWithImplicit<StringRef, std::weak_ordering> {
public:
  StringRef() noexcept = default;
  StringRef(StringRef const& string) noexcept = default;
  StringRef(StringRef&& string) noexcept = default;
  ~StringRef() noexcept = default;
  explicit(false) StringRef(cds::String const& string) noexcept;
  explicit(false) StringRef(cds::StringView const& string) noexcept;
  explicit(false) StringRef(std::string const& string) noexcept;
  explicit(false) StringRef(std::string_view const& string) noexcept;
  explicit(false) StringRef(char const* string) noexcept;
  StringRef(char const* string, cds::Size length) noexcept;

  template <int size> explicit(false) StringRef(char const (&string)[size]) noexcept : StringRef(string, size) {}

  auto operator=(StringRef const& string) noexcept -> StringRef& = default;
  auto operator=(StringRef&& string) noexcept -> StringRef& = default;
  auto operator=(cds::String const& string) noexcept -> StringRef&;
  auto operator=(cds::StringView const& string) noexcept -> StringRef&;
  auto operator=(std::string const& string) noexcept -> StringRef&;
  auto operator=(std::string_view const& string) noexcept -> StringRef&;
  auto operator=(char const* string) noexcept -> StringRef&;

  template <int size> auto operator=(char const (&string)[size]) noexcept -> StringRef& {
    return operator=(StringRef(string));
  }

  [[nodiscard]] explicit(false) operator bool() const noexcept;
  [[nodiscard]] explicit(false) operator cds::StringView() const noexcept;
  [[nodiscard]] explicit(false) operator cds::String() const noexcept;

  auto clear() noexcept -> void;
  [[nodiscard]] auto takeFront(cds::Size amount) const noexcept -> StringRef;
  [[nodiscard]] auto takeBack(cds::Size amount) const noexcept -> StringRef;
  [[nodiscard]] auto dropFront(cds::Size amount) const noexcept -> StringRef;
  [[nodiscard]] auto dropBack(cds::Size amount) const noexcept -> StringRef;
  auto shrink(cds::Size amount) noexcept -> StringRef&;

  [[nodiscard]] auto find(char character) const noexcept -> cds::Index;

  [[nodiscard]] auto sub(cds::Size offset, cds::Size length) const noexcept -> StringRef;

  [[nodiscard]] auto operator+(StringRef const& ref) const noexcept -> cds::String;
  using ::age::meta::op::AddWithImplicit<StringRef>::operator+;

  [[nodiscard]] auto operator<=>(StringRef const& ref) const noexcept -> std::weak_ordering;

  [[nodiscard]] constexpr auto data() const noexcept { return _buffer; }
  [[nodiscard]] constexpr auto size() const noexcept { return _size; }
  [[nodiscard]] constexpr auto empty() const noexcept -> bool { return data() == nullptr || size() == 0u; }

  static constexpr cds::Index const npos = cds::String::invalidIndex;

private:
  char const* _buffer {nullptr};
  cds::Size _size {0u};
  using Utils = cds::StringUtils<char>;
};
} // namespace age
