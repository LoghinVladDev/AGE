//
// Created by Vlad-Andrei Loghin on 23.05.23.
//

#pragma once

#include <CDS/meta/TypeTraits>

namespace age::meta {

/// \brief Meta-Type used to identify whether a given specialized Enum type is a Flag Enum to generate biwise flag operators for
template <typename Enum>
  requires(cds::meta::IsEnum<Enum>::value)
struct FlagEnum : cds::meta::FalseType {
  using Type = Enum;
  using FlagsType = std::underlying_type_t<Enum>;
};

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator|(T left, T right) noexcept -> FlagEnum<T>::FlagsType {
  using Underlying = std::underlying_type_t<T>;
  return static_cast<Underlying>(left) | static_cast<Underlying>(right);
}

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator|(T left, typename FlagEnum<T>::FlagsType right) noexcept -> FlagEnum<T>::FlagsType {
  using Underlying = std::underlying_type_t<T>;
  return static_cast<Underlying>(left) | static_cast<Underlying>(right);
}

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator|(typename FlagEnum<T>::FlagsType left, T right) noexcept -> FlagEnum<T>::FlagsType {
  using Underlying = std::underlying_type_t<T>;
  return static_cast<Underlying>(left) | static_cast<Underlying>(right);
}

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator&(T left, T right) noexcept -> FlagEnum<T>::FlagsType {
  using Underlying = std::underlying_type_t<T>;
  return static_cast<Underlying>(left) & static_cast<Underlying>(right);
}

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator&(T left, typename FlagEnum<T>::FlagsType right) noexcept -> FlagEnum<T>::FlagsType {
  using Underlying = std::underlying_type_t<T>;
  return static_cast<Underlying>(left) & static_cast<Underlying>(right);
}

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator&(typename FlagEnum<T>::FlagsType left, T right) noexcept -> FlagEnum<T>::FlagsType {
  using Underlying = std::underlying_type_t<T>;
  return static_cast<Underlying>(left) & static_cast<Underlying>(right);
}

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator^(T left, T right) noexcept -> FlagEnum<T>::FlagsType {
  using Underlying = std::underlying_type_t<T>;
  return static_cast<Underlying>(left) ^ static_cast<Underlying>(right);
}

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator^(T left, typename FlagEnum<T>::FlagsType right) noexcept -> FlagEnum<T>::FlagsType {
  using Underlying = std::underlying_type_t<T>;
  return static_cast<Underlying>(left) ^ static_cast<Underlying>(right);
}

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator^(typename FlagEnum<T>::FlagsType left, T right) noexcept -> FlagEnum<T>::FlagsType {
  using Underlying = std::underlying_type_t<T>;
  return static_cast<Underlying>(left) ^ static_cast<Underlying>(right);
}

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator~(T operand) noexcept -> FlagEnum<T>::FlagsType {
  return ~static_cast<std::underlying_type_t<T>>(operand);
}

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator|=(typename FlagEnum<T>::FlagsType& left, T right) noexcept -> FlagEnum<T>::FlagsType& {
  using Underlying = std::underlying_type_t<T>;
  left |= static_cast<Underlying>(right);
  return left;
}

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator&=(typename FlagEnum<T>::FlagsType& left, T right) noexcept -> FlagEnum<T>::FlagsType& {
  using Underlying = std::underlying_type_t<T>;
  left &= static_cast<Underlying>(right);
  return left;
}

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator^=(typename FlagEnum<T>::FlagsType& left, T right) noexcept -> FlagEnum<T>::FlagsType& {
  using Underlying = std::underlying_type_t<T>;
  left ^= static_cast<Underlying>(right);
  return left;
}

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator==(T left, typename FlagEnum<T>::FlagsType right) noexcept -> bool {
  return static_cast<std::underlying_type_t<T>>(left) == right;
}

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator==(typename FlagEnum<T>::FlagsType left, T right) noexcept -> bool {
  return left == static_cast<std::underlying_type_t<T>>(right);
}

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator!=(T left, typename FlagEnum<T>::FlagsType right) noexcept -> bool {
  return static_cast<std::underlying_type_t<T>>(left) != right;
}

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator!=(typename FlagEnum<T>::FlagsType left, T right) noexcept -> bool {
  return left != static_cast<std::underlying_type_t<T>>(right);
}

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator>(T left, typename FlagEnum<T>::FlagsType right) noexcept -> bool {
  return static_cast<std::underlying_type_t<T>>(left) > right;
}

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator>(typename FlagEnum<T>::FlagsType left, T right) noexcept -> bool {
  return left > static_cast<std::underlying_type_t<T>>(right);
}

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator<(T left, typename FlagEnum<T>::FlagsType right) noexcept -> bool {
  return static_cast<std::underlying_type_t<T>>(left) < right;
}

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator<(typename FlagEnum<T>::FlagsType left, T right) noexcept -> bool {
  return left < static_cast<std::underlying_type_t<T>>(right);
}

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator>=(T left, typename FlagEnum<T>::FlagsType right) noexcept -> bool {
  return static_cast<std::underlying_type_t<T>>(left) >= right;
}

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator>=(typename FlagEnum<T>::FlagsType left, T right) noexcept -> bool {
  return left >= static_cast<std::underlying_type_t<T>>(right);
}

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator<=(T left, typename FlagEnum<T>::FlagsType right) noexcept -> bool {
  return static_cast<std::underlying_type_t<T>>(left) <= right;
}

template <typename T>
  requires(FlagEnum<T>::value)
constexpr auto operator<=(typename FlagEnum<T>::FlagsType left, T right) noexcept -> bool {
  return left <= static_cast<std::underlying_type_t<T>>(right);
}
} // namespace age::meta
