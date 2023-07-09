//
// Created by Vlad-Andrei Loghin on 09.07.23.
//

#include <filesystem>
#include <gtest/gtest.h>
#include <lang/filesystem/PathAwareFstream.hpp>

namespace {
using age::PathAwareFstream;
using age::PathAwareOfstream;

constexpr auto sampleData = R"(
Here
is
some
sample data.
)";
} // namespace

template <typename StreamType, typename... A> auto outputToFile(auto data, A&&... a) {
  StreamType file(".temp", std::forward<A>(a)...);
  file << data;
}

auto validateAndRemove(auto data) {
  std::ifstream file(".temp");
  std::stringstream stream;
  stream << file.rdbuf();
  file.close();
  std::filesystem::remove(".temp");
  return stream.str() == data;
}

TEST(PathAwareFstream, fstream) {
  outputToFile<PathAwareOfstream>(sampleData);
  ASSERT_TRUE(validateAndRemove(sampleData));

  outputToFile<PathAwareFstream>(sampleData, std::ios::out);
  ASSERT_TRUE(validateAndRemove(sampleData));
}

TEST(PathAwareFstream, fstreamCreate) {
  PathAwareOfstream out(".temp/.nested/.temp");
  out << "test";
  out.close();
  PathAwareFstream in(".temp/.nested/.temp");
  std::stringstream stream;
  stream << in.rdbuf();
  ASSERT_EQ(stream.str(), "test");
  std::filesystem::remove_all(".temp");
}
