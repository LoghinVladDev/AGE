//
// Created by Vlad-Andrei Loghin on 07.07.23.
//

#pragma once

namespace age {
#ifdef WIN32
constexpr char const directorySeparator = '\\';
#else
constexpr char const directorySeparator = '/';
#endif
} // namespace age
