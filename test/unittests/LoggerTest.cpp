//
// Created by loghin on 8/21/23.
//

#include <gtest/gtest.h>

#include <core/logging/Logger.hpp>

namespace {
using age::Logger;
} // namespace

#ifndef NDEBUG
TEST(LoggerTest, basicOut) {
  std::stringstream outbuf;
  auto logger = Logger::get(outbuf);

  logger() << "basic string output, followed by numeric: " << 123 << std::hex << 15 << std::dec << 24;
}
#endif
