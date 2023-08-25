//
// Created by loghin on 8/21/23.
//

#include <gtest/gtest.h>

#include <core/logging/Logger.hpp>

namespace {
using age::Logger;
using namespace std;

template <typename T> auto contains(stringstream const& stream, T&& what) {
  return stream.str().find(std::forward<T>(what)) != string::npos;
}
} // namespace

#ifndef NDEBUG
TEST(LoggerTest, basicOut) {
  stringstream outbuf;
  auto logger = Logger::get(cout);

  logger() << "basic string output, followed by numeric: " << 123 << std::hex << 15 << std::dec << 24;
}
#endif
