//
// Created by Vlad-Andrei Loghin on 07.07.23.
//

#ifndef AGE_PATH_UTILS_HPP
#define AGE_PATH_UTILS_HPP

namespace age {
#ifdef WIN32
constexpr char const directorySeparator = '\\';
#else
constexpr char const directorySeparator = '/';
#endif
} // namespace age

#endif // AGE_PATH_UTILS_HPP
