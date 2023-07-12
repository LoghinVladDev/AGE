//
// Created by Vlad-Andrei Loghin on 10.07.23.
//

#pragma once

#include <CDS/Types>

namespace age::visualizer::meta {
enum class ActionBinding : cds::uint32 {
#define ACTION(name) AB_##name,
  AB_nullAction = 0x80000000u,
#include "ActionBindings.inc"
#undef ACTION
};

auto actionId(ActionBinding binding) { return static_cast<std::underlying_type_t<ActionBinding>>(binding); }
} // namespace age::visualizer::meta
