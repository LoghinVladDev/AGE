//
// Created by Vlad-Andrei Loghin on 01.07.23.
//

#pragma once
#include <compare>

namespace age::meta::op {
/// Required for avoiding any implicit conversions to other types when invoking == based on <=>
template <typename Base, typename Ordering> class GenFromSpaceship {
public:
  [[nodiscard]] auto operator==(GenFromSpaceship const& object) const noexcept {
    return static_cast<Base const*>(this)->operator<=>(static_cast<Base const&>(object)) == Ordering::equivalent;
  }
};
} // namespace age::meta::op
