//
// Created by Vlad-Andrei Loghin on 27.07.23.
//

#include <core/lang/generic/Concepts.hpp>

namespace {
using namespace age::meta::concepts;
} // namespace

consteval auto forcedConstexprFunction() {
  static_assert(SameAs<int, int>);
  static_assert(!SameAs<int, float>);
  return 0;
}

void f() { (void) forcedConstexprFunction(); }
