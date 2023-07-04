//
// Created by Vlad-Andrei Loghin on 01.07.23.
//

#ifndef AGE_OPERATORS_WITH_IMPLICIT_CONSTRUCTIONS_HPP
#define AGE_OPERATORS_WITH_IMPLICIT_CONSTRUCTIONS_HPP

namespace age::meta::op {
template <typename L, typename R, typename = void> struct AmbiguousAdd : cds::meta::FalseType {};
template <typename L, typename R, typename = void> struct AmbiguousSpaceship : cds::meta::FalseType {};
template <typename L, typename R, typename = void> struct AmbiguousEquals : cds::meta::FalseType {};

template <typename L, typename R> struct AmbiguousAdd<
    L, R,
    cds::meta::Void<decltype(cds::meta::valueOf<cds::meta::Decay<L>>() + cds::meta::valueOf<cds::meta::Decay<R>>())>> :
    cds::meta::TrueType {};

template <typename L, typename R>
struct AmbiguousSpaceship<L, R,
                          cds::meta::Void<decltype(cds::meta::valueOf<cds::meta::Decay<L>>()
                                                   <=> cds::meta::valueOf<cds::meta::Decay<R>>())>> :
    cds::meta::TrueType {};

template <typename L, typename R> struct AmbiguousEquals<
    L, R,
    cds::meta::Void<decltype(cds::meta::valueOf<cds::meta::Decay<L>>() == cds::meta::valueOf<cds::meta::Decay<R>>())>> :
    cds::meta::TrueType {};

template <typename L, typename R> constexpr auto ambiguousAdd = AmbiguousAdd<L, R>::value;
template <typename L, typename R> constexpr auto ambiguousSpaceship = AmbiguousSpaceship<L, R>::value;
template <typename L, typename R> constexpr auto ambiguousEquals = AmbiguousEquals<L, R>::value;

template <typename Base> class AddWithImplicit {
public:
  template <typename C, cds::meta::EnableIf<cds::meta::Not<cds::meta::IsSame<Base, cds::meta::Decay<C>>>::value> = 0>
  [[nodiscard]] auto operator+(C&& convertible) const noexcept {
    return static_cast<Base const*>(this)->operator+(Base(std::forward<C>(convertible)));
  }

  template <typename C, cds::meta::EnableIf<!AmbiguousAdd<C, Base>::value> = 0>
  [[nodiscard]] friend auto operator+(C&& convertible, Base const& base) noexcept {
    return Base(std::forward<C>(convertible)) + base;
  }
};

template <typename Base, typename Ordering> class SpaceshipWithImplicit {
public:
  template <typename C, cds::meta::EnableIf<cds::meta::Not<cds::meta::IsSame<Base, cds::meta::Decay<C>>>::value> = 0>
  [[nodiscard]] auto operator<=>(C&& convertible) const noexcept {
    return static_cast<Base const*>(this)->operator<=>(Base(std::forward<C>(convertible)));
  }

  template <typename C, cds::meta::EnableIf<cds::meta::Not<cds::meta::IsSame<Base, cds::meta::Decay<C>>>::value> = 0>
  [[nodiscard]] auto operator==(C&& object) const noexcept -> bool {
    return operator<=>(std::forward<C>(object)) == Ordering::equivalent;
  }

  template <typename C, cds::meta::EnableIf<cds::meta::Not<cds::meta::IsSame<Base, cds::meta::Decay<C>>>::value> = 0>
  [[nodiscard]] friend auto operator<(C&& object, Base const& base) noexcept {
    return (Base(std::forward<C>(object)) <=> base) == Ordering::less;
  }

  template <typename C, cds::meta::EnableIf<cds::meta::Not<cds::meta::IsSame<Base, cds::meta::Decay<C>>>::value> = 0>
  [[nodiscard]] friend auto operator>(C&& object, Base const& base) noexcept {
    return (Base(std::forward<C>(object)) <=> base) == Ordering::greater;
  }

  template <typename C, cds::meta::EnableIf<cds::meta::Not<cds::meta::IsSame<Base, cds::meta::Decay<C>>>::value> = 0>
  [[nodiscard]] friend auto operator<=(C&& object, Base const& base) noexcept {
    auto res = (Base(std::forward<C>(object)) <=> base);
    return res == Ordering::less || res == Ordering::equivalent;
  }

  template <typename C, cds::meta::EnableIf<cds::meta::Not<cds::meta::IsSame<Base, cds::meta::Decay<C>>>::value> = 0>
  [[nodiscard]] friend auto operator>=(C&& object, Base const& base) noexcept {
    auto res = (Base(std::forward<C>(object)) <=> base);
    return res == Ordering::greater || res == Ordering::equivalent;
  }
};
} // namespace age::meta::op

#endif // AGE_OPERATORS_WITH_IMPLICIT_CONSTRUCTIONS_HPP
