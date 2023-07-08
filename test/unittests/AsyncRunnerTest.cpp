//
// Created by Vlad-Andrei Loghin on 09.07.23.
//

#include <gtest/gtest.h>
#include <lang/thread/AsyncRunner.hpp>

namespace {
using age::AsyncRunner;
} // namespace

TEST(AsyncRunnerTest, voidAsync) {
  AsyncRunner<void, int*, int const*> intCopyAsyncRunner = [](int* d, int const* s) { *d = *s; };

  int src = 3;
  int dst = 0;
  intCopyAsyncRunner.trigger(&dst, &src);
  intCopyAsyncRunner.await();

  ASSERT_EQ(dst, 3);
}

TEST(AsyncRunnerTest, returningAsync) {
  AsyncRunner<cds::String, int> intToStringAsync = [](int x) { return std::to_string(x); };

  ASSERT_TRUE(intToStringAsync.notStarted());
  intToStringAsync.trigger(5);
  ASSERT_FALSE(intToStringAsync.notStarted());
  auto result = intToStringAsync.await();
  ASSERT_EQ(result, "5");
}
