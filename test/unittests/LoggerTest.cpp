//
// Created by loghin on 8/21/23.
//

#include <gtest/gtest.h>

#include <CDS/Function>
#include <CDS/threading/Thread>

#include <core/logging/Logger.hpp>

namespace {
using age::Logger;
using age::LoggerOutput;
using age::StringRef;

using cds::Function;
using cds::String;
using cds::Thread;

using namespace std;

using Validator = Function<bool(String&)>;

struct Glob {
  static inline thread_local source_location sourceLocation;
  static inline thread_local bool prefix;
  static inline thread_local StringRef name;
  static inline thread_local int lineNo;
};

template <typename T> auto contains(stringstream const& stream, T&& what) {
  return stream.str().find(std::forward<T>(what)) != string::npos;
}

template <typename T> auto contains(string const& stream, T&& what) {
  return stream.find(std::forward<T>(what)) != string::npos;
}

auto flagTest() {
  stringstream outbuf;
  auto logger = Logger::get(outbuf);
  logger.disableOptions(Logger::defaultOptionFlags);
  logger() << "test";
#ifdef NDEBUG
  return true;
#endif
  return outbuf.str() == "test\n";
}

template <Logger::OptionFlag flag> auto validator() -> Validator;

auto findMatching(auto& buf, char what, char matching) {
  auto pairs = 0;
  for (cds::Index i = 0; i < buf.length(); ++i) {
    if (buf[i] == matching) {
      ++pairs;
    } else if (buf[i] == what) {
      --pairs;
      if (pairs == 0) {
        return i;
      }
    }
  }
  return StringRef::npos;
}

auto isolateAndRemoveMeta(auto& buf, auto const& what) {
  auto remaining = buf;
  auto l = remaining.findFirst('[');
  auto r = findMatching(remaining, ']', '[');
  auto o = 0;
  while (!remaining.empty() && l != StringRef::npos && r != StringRef::npos) {
    if (auto section = buf.substr(l + o, r + o + 1); section == StringRef("[") + what + "]") {
      auto ls = buf.substr(o, l + o);
      auto rs = buf.substr(r + o + 1);
      /// TODO: cds::String::append(String&&) with empty string causes nullptr pass to memcpy
      buf = (age::ref(ls) ? age::ref(ls) : age::ref("")) + (age::ref(rs) ? age::ref(rs) : age::ref(""));
      return true;
    }

    o += r;
    remaining = remaining.substr(r + 1);
    l = remaining.findFirst('[');
    r = findMatching(remaining, ']', '[');
  }
  return false;
}

auto validateFlags(String& buf, auto&& lastValidator) { return lastValidator(buf); }

auto validateFlags(String& buf, auto&& firstValidator, auto&&... remainingValidators) {
  return firstValidator(buf) && validateFlags(buf, remainingValidators...);
}

template <Logger::OptionFlag... flags> auto flagTest() {
  Glob::prefix = false;
  stringstream outbuf;
  auto logger = Logger::get(outbuf);
  Glob::name = logger.name();
  logger.setOptions((flags | ...));
  logger() << "test";
  Glob::lineNo = source_location().line() - 1;
  Glob::sourceLocation = source_location();
  auto buf = String(outbuf.str());
#ifdef NDEBUG
  return true;
#endif
  return validateFlags(buf, validator<flags>()...) && buf == " test\n";
}

template <> Validator validator<Logger::OptionFlag::SourceLocation>() {
  return [](String& buf) {
    return isolateAndRemoveMeta(buf, age::ref(Glob::sourceLocation.file_name()) + ":" + Glob::sourceLocation.line());
  };
}

template <> Validator validator<Logger::OptionFlag::SourceLocationFile>() {
  return [](String& buf) { return isolateAndRemoveMeta(buf, age::ref(Glob::sourceLocation.file_name())); };
}

template <> Validator validator<Logger::OptionFlag::SourceLocationLine>() {
  return [](String& buf) { return isolateAndRemoveMeta(buf, age::ref(std::to_string(Glob::lineNo))); };
}

template <> Validator validator<Logger::OptionFlag::SourceLocationFunction>() {
  return [](String& buf) { return isolateAndRemoveMeta(buf, Glob::sourceLocation.function_name()); };
}

template <> Validator validator<Logger::OptionFlag::InfoPrefix>() {
  return [](String const&) {
    Glob::prefix = true;
    return true;
  };
}

auto conditionedPrefix(auto prefixStr, auto after) {
  return (Glob::prefix ? age::ref((age::ref(prefixStr) + " = ")) : age::ref("")) + after;
}

template <> Validator validator<Logger::OptionFlag::LoggerName>() {
  return [](String& buf) { return isolateAndRemoveMeta(buf, conditionedPrefix("logger", Glob::name)); };
}

template <> Validator validator<Logger::OptionFlag::LogLevel>() {
  return [](String& buf) { return isolateAndRemoveMeta(buf, conditionedPrefix("level", "Info")); };
}

template <> Validator validator<Logger::OptionFlag::ThreadId>() {
  return [](String& buf) {
    stringstream thbuf;
    thbuf << "0x" << std::hex << Thread::currentThreadID();
    return isolateAndRemoveMeta(buf, conditionedPrefix("thread", thbuf.str()));
  };
}

template <> Validator validator<Logger::OptionFlag::OutputTerminalColour>() {
  return [](String const&) {
    // do nothing, no colour validation on non-terminal output
    return true;
  };
}

auto filterIndividual() {
  using enum Logger::Level;
  std::stringstream inf;
  std::stringstream wrn;
  std::stringstream err;
  std::stringstream dbg;
  auto logger = Logger::get(LoggerOutput {inf, Info}, LoggerOutput {wrn, Warning}, LoggerOutput {err, Error},
                            LoggerOutput {dbg, Debug});

  logger(Info) << "test1";
  logger(Warning) << "test2";
  logger(Error) << "test3";
  logger(Debug) << "test4";
  return std::make_tuple(inf.str(), wrn.str(), err.str(), dbg.str());
}

auto filterMixedOut() {
  using enum Logger::Level;
  std::stringstream infWrn;
  std::stringstream wrnErr;
  std::stringstream errDbg;
  auto logger = Logger::get(LoggerOutput {infWrn, Info | Warning}, LoggerOutput {wrnErr, Warning | Error},
                            LoggerOutput {errDbg, Error | Debug});

  logger(Info) << "test1";
  logger(Warning) << "test2";
  logger(Error) << "test3";
  logger(Debug) << "test4";
  return std::make_tuple(infWrn.str(), wrnErr.str(), errDbg.str());
}
} // namespace

#ifndef NDEBUG
TEST(LoggerTest, basicOut) {
  stringstream outbuf;
  auto logger = Logger::get(outbuf);
  logger() << "basic string output, followed by numeric: " << 123 << std::hex << 15 << std::dec << 24;
  ASSERT_TRUE(contains(outbuf, "basic string output, followed by numeric: 123f24"));
}

#define WRAP_ASSERT_TRUE(...)                                                                                          \
  do {                                                                                                                 \
    auto v = __VA_ARGS__;                                                                                              \
    ASSERT_TRUE(v);                                                                                                    \
  } while (false)

TEST(LoggerTest, metainfo) {
  using O = Logger::OptionFlag;
  ASSERT_TRUE(flagTest());
  ASSERT_TRUE(flagTest<O::SourceLocation>());
  ASSERT_TRUE(flagTest<O::SourceLocationFile>());
  ASSERT_TRUE(flagTest<O::SourceLocationLine>());
  ASSERT_TRUE(flagTest<O::SourceLocationFunction>());
  ASSERT_TRUE(flagTest<O::LoggerName>());
  WRAP_ASSERT_TRUE(flagTest<O::InfoPrefix, O::LoggerName>());
  ASSERT_TRUE(flagTest<O::LogLevel>());
  WRAP_ASSERT_TRUE(flagTest<O::InfoPrefix, O::LogLevel>());
  ASSERT_TRUE(flagTest<O::ThreadId>());
  WRAP_ASSERT_TRUE(flagTest<O::InfoPrefix, O::ThreadId>());

  WRAP_ASSERT_TRUE(flagTest<O::SourceLocation, O::LoggerName>());
  WRAP_ASSERT_TRUE(flagTest<O::SourceLocation, O::LogLevel>());
  WRAP_ASSERT_TRUE(flagTest<O::SourceLocation, O::LoggerName, O::LogLevel>());

  WRAP_ASSERT_TRUE(flagTest<O::SourceLocation, O::InfoPrefix, O::LoggerName>());
  WRAP_ASSERT_TRUE(flagTest<O::SourceLocation, O::InfoPrefix, O::LogLevel>());
  WRAP_ASSERT_TRUE(flagTest<O::SourceLocation, O::InfoPrefix, O::LoggerName, O::LogLevel>());

  WRAP_ASSERT_TRUE(flagTest<O::SourceLocation, O::LoggerName, O::ThreadId>());
  WRAP_ASSERT_TRUE(flagTest<O::SourceLocation, O::LogLevel, O::ThreadId>());
  WRAP_ASSERT_TRUE(flagTest<O::SourceLocation, O::LoggerName, O::LogLevel, O::ThreadId>());

  WRAP_ASSERT_TRUE(flagTest<O::SourceLocation, O::InfoPrefix, O::LoggerName, O::ThreadId>());
  WRAP_ASSERT_TRUE(flagTest<O::SourceLocation, O::InfoPrefix, O::LogLevel, O::ThreadId>());
  WRAP_ASSERT_TRUE(flagTest<O::SourceLocation, O::InfoPrefix, O::LoggerName, O::LogLevel, O::ThreadId>());

  WRAP_ASSERT_TRUE(flagTest<O::OutputTerminalColour, O::SourceLocation, O::LoggerName>());
  WRAP_ASSERT_TRUE(flagTest<O::OutputTerminalColour, O::SourceLocation, O::LogLevel>());
  WRAP_ASSERT_TRUE(flagTest<O::OutputTerminalColour, O::SourceLocation, O::LoggerName, O::LogLevel>());

  WRAP_ASSERT_TRUE(flagTest<O::OutputTerminalColour, O::SourceLocation, O::InfoPrefix, O::LoggerName>());
  WRAP_ASSERT_TRUE(flagTest<O::OutputTerminalColour, O::SourceLocation, O::InfoPrefix, O::LogLevel>());
  WRAP_ASSERT_TRUE(flagTest<O::OutputTerminalColour, O::SourceLocation, O::InfoPrefix, O::LoggerName, O::LogLevel>());

  WRAP_ASSERT_TRUE(flagTest<O::OutputTerminalColour, O::SourceLocation, O::LoggerName, O::ThreadId>());
  WRAP_ASSERT_TRUE(flagTest<O::OutputTerminalColour, O::SourceLocation, O::LogLevel, O::ThreadId>());
  WRAP_ASSERT_TRUE(flagTest<O::OutputTerminalColour, O::SourceLocation, O::LoggerName, O::LogLevel, O::ThreadId>());

  WRAP_ASSERT_TRUE(flagTest<O::OutputTerminalColour, O::SourceLocation, O::InfoPrefix, O::LoggerName, O::ThreadId>());
  WRAP_ASSERT_TRUE(flagTest<O::OutputTerminalColour, O::SourceLocation, O::InfoPrefix, O::LogLevel, O::ThreadId>());
  WRAP_ASSERT_TRUE(
      flagTest<O::OutputTerminalColour, O::SourceLocation, O::InfoPrefix, O::LoggerName, O::LogLevel, O::ThreadId>());
}

TEST(LoggerTest, levelSwitch) {
  stringstream outbuf;
  auto logger = Logger::get(outbuf);

  logger.setOptions(Logger::OptionFlag::LogLevel);

  auto testImplicitLevel = [&outbuf, &logger](Logger::Level expectedLevel) {
    stringstream().swap(outbuf);
    logger() << "12345678";
    ASSERT_TRUE(contains(outbuf, "12345678"));
    ASSERT_TRUE(expectedLevel == Logger::Level::Info ? contains(outbuf, "Info") : !contains(outbuf, "Info"));
    ASSERT_TRUE(expectedLevel == Logger::Level::Warning ? contains(outbuf, "Warning") : !contains(outbuf, "Warning"));
    ASSERT_TRUE(expectedLevel == Logger::Level::Debug ? contains(outbuf, "Debug") : !contains(outbuf, "Debug"));
    ASSERT_TRUE(expectedLevel == Logger::Level::Error ? contains(outbuf, "Error") : !contains(outbuf, "Error"));
  };

  auto testExplicitLevel = [&outbuf, &logger](Logger::Level level) {
    stringstream().swap(outbuf);
    logger(level) << "12345678";
    ASSERT_TRUE(contains(outbuf, "12345678"));
    ASSERT_TRUE(level == Logger::Level::Info ? contains(outbuf, "Info") : !contains(outbuf, "Info"));
    ASSERT_TRUE(level == Logger::Level::Warning ? contains(outbuf, "Warning") : !contains(outbuf, "Warning"));
    ASSERT_TRUE(level == Logger::Level::Debug ? contains(outbuf, "Debug") : !contains(outbuf, "Debug"));
    ASSERT_TRUE(level == Logger::Level::Error ? contains(outbuf, "Error") : !contains(outbuf, "Error"));
  };

  auto testSwitch = [&testImplicitLevel, &testExplicitLevel](Logger::Level expectedImplicit) {
    testImplicitLevel(expectedImplicit);
    testExplicitLevel(Logger::Level::Warning);
    testImplicitLevel(expectedImplicit);
    testExplicitLevel(Logger::Level::Debug);
    testImplicitLevel(expectedImplicit);
    testExplicitLevel(Logger::Level::Error);
    testImplicitLevel(expectedImplicit);
    testExplicitLevel(Logger::Level::Info);
    testImplicitLevel(expectedImplicit);
  };

  testSwitch(Logger::Level::Info);
  logger.setDefaultLevel(Logger::Level::Warning);
  testSwitch(Logger::Level::Warning);
  logger.setDefaultLevel(Logger::Level::Error);
  testSwitch(Logger::Level::Error);
  logger.setDefaultLevel(Logger::Level::Debug);
  testSwitch(Logger::Level::Debug);
  logger.setDefaultLevel(Logger::Level::Info);
  testSwitch(Logger::Level::Info);
}

TEST(LoggerTest, defaultOut) {
  stringstream out;
  auto logger = Logger::get();
  logger.disableOptions(Logger::defaultOptionFlags);

  logger() << "<Text outputted part of test, to be ignored>";
  ASSERT_TRUE(out.str().empty());

  logger.enableOptions(Logger::OptionFlag::OutputTerminalColour);
  logger(Logger::Level::Info) << "<Text outputted part of test, to be ignored>";
  ASSERT_TRUE(out.str().empty());
  logger(Logger::Level::Warning) << "<Text outputted part of test, to be ignored>";
  ASSERT_TRUE(out.str().empty());
  logger(Logger::Level::Debug) << "<Text outputted part of test, to be ignored>";
  ASSERT_TRUE(out.str().empty());
  logger(Logger::Level::Error) << "<Text outputted part of test, to be ignored>";
  ASSERT_TRUE(out.str().empty());

  Logger::setDefaultOutput(out);
  auto newLogger = Logger::get();
  newLogger() << "<Text outputted part of test, to be ignored>";
  ASSERT_TRUE(contains(out, "<Text outputted part of test, to be ignored>"));
}

TEST(LoggerTest, endLining) {
  stringstream out;
  auto logger = Logger::get(out);

  std::cout << std::endl;
  logger() << "test" << std::endl << "another";
  ASSERT_FALSE(out.str().empty());
  ASSERT_TRUE(contains(out, "test\nanother"));
}

TEST(LoggerTest, individual) {
  auto [inf, wrn, err, dbg] = filterIndividual();

  ASSERT_TRUE(contains(inf, "Info"));
  ASSERT_TRUE(contains(inf, "test1"));
  ASSERT_FALSE(contains(inf, "Warning"));
  ASSERT_FALSE(contains(inf, "test2"));
  ASSERT_FALSE(contains(inf, "Error"));
  ASSERT_FALSE(contains(inf, "test3"));
  ASSERT_FALSE(contains(inf, "Debug"));
  ASSERT_FALSE(contains(inf, "test4"));

  ASSERT_FALSE(contains(wrn, "Info"));
  ASSERT_FALSE(contains(wrn, "test1"));
  ASSERT_TRUE(contains(wrn, "Warning"));
  ASSERT_TRUE(contains(wrn, "test2"));
  ASSERT_FALSE(contains(wrn, "Error"));
  ASSERT_FALSE(contains(wrn, "test3"));
  ASSERT_FALSE(contains(wrn, "Debug"));
  ASSERT_FALSE(contains(wrn, "test4"));

  ASSERT_FALSE(contains(err, "Info"));
  ASSERT_FALSE(contains(err, "test1"));
  ASSERT_FALSE(contains(err, "Warning"));
  ASSERT_FALSE(contains(err, "test2"));
  ASSERT_TRUE(contains(err, "Error"));
  ASSERT_TRUE(contains(err, "test3"));
  ASSERT_FALSE(contains(err, "Debug"));
  ASSERT_FALSE(contains(err, "test4"));

  ASSERT_FALSE(contains(dbg, "Info"));
  ASSERT_FALSE(contains(dbg, "test1"));
  ASSERT_FALSE(contains(dbg, "Warning"));
  ASSERT_FALSE(contains(dbg, "test2"));
  ASSERT_FALSE(contains(dbg, "Error"));
  ASSERT_FALSE(contains(dbg, "test3"));
  ASSERT_TRUE(contains(dbg, "Debug"));
  ASSERT_TRUE(contains(dbg, "test4"));
}

TEST(LoggerTest, mixedOut) {
  auto [infWrn, wrnErr, errDbg] = filterMixedOut();

  ASSERT_TRUE(contains(infWrn, "Info"));
  ASSERT_TRUE(contains(infWrn, "test1"));
  ASSERT_TRUE(contains(infWrn, "Warning"));
  ASSERT_TRUE(contains(infWrn, "test2"));
  ASSERT_FALSE(contains(infWrn, "Error"));
  ASSERT_FALSE(contains(infWrn, "test3"));
  ASSERT_FALSE(contains(infWrn, "Debug"));
  ASSERT_FALSE(contains(infWrn, "test4"));

  ASSERT_FALSE(contains(wrnErr, "Info"));
  ASSERT_FALSE(contains(wrnErr, "test1"));
  ASSERT_TRUE(contains(wrnErr, "Warning"));
  ASSERT_TRUE(contains(wrnErr, "test2"));
  ASSERT_TRUE(contains(wrnErr, "Error"));
  ASSERT_TRUE(contains(wrnErr, "test3"));
  ASSERT_FALSE(contains(wrnErr, "Debug"));
  ASSERT_FALSE(contains(wrnErr, "test4"));

  ASSERT_FALSE(contains(errDbg, "Info"));
  ASSERT_FALSE(contains(errDbg, "test1"));
  ASSERT_FALSE(contains(errDbg, "Warning"));
  ASSERT_FALSE(contains(errDbg, "test2"));
  ASSERT_TRUE(contains(errDbg, "Error"));
  ASSERT_TRUE(contains(errDbg, "test3"));
  ASSERT_TRUE(contains(errDbg, "Debug"));
  ASSERT_TRUE(contains(errDbg, "test4"));
}

TEST(LoggerTest, multiOutBasic) {
  stringstream out1;
  stringstream out2;
  auto logger = Logger::get(out1, out2);
  logger() << "basic string output, followed by numeric: " << 123 << std::hex << 15 << std::dec << 24;
  ASSERT_EQ(out1.str(), out2.str());
  ASSERT_TRUE(contains(out1.str(), "basic string output, followed by numeric: 123f24"));
  ASSERT_TRUE(contains(out2.str(), "basic string output, followed by numeric: 123f24"));
}

TEST(LoggerTest, isolation) {
  stringstream out1;
  stringstream out2;
  auto logger = Logger::get(out1, out2);

  logger() << "test";
  stringstream().swap(out2);
  ASSERT_FALSE(out1.str().empty());
  ASSERT_TRUE(out2.str().empty());
  ASSERT_TRUE(contains(out1, "test"));
  ASSERT_FALSE(contains(out2, "test"));
}

TEST(LoggerTest, constOutputs) {
  stringstream sb1;
  stringstream sb2;
  stringstream sb3;
  auto l = Logger::get();
  l.outputs() = {sb1, sb2};
  auto const& cl = l;
  ASSERT_EQ(cl.outputs().size(), 2);
  ASSERT_TRUE(cl.outputs().any([&sb1](auto const& l) { return &l.output() == &sb1; }));
  ASSERT_TRUE(cl.outputs().any([&sb2](auto const& l) { return &l.output() == &sb2; }));
  ASSERT_FALSE(cl.outputs().any([&sb3](auto const& l) { return &l.output() == &sb3; }));
}

TEST(LoggerTest, naming) {
  auto const& logger = Logger::get("testLog");
  ASSERT_EQ(logger.name(), "testLog");
}

TEST(LoggerTest, anonymousLogger) {
  stringstream outbuf1;
  auto logger1 = Logger::get(outbuf1);
  stringstream outbuf2;
  auto logger2 = Logger::get(outbuf2);
  auto logger3 = Logger::get();

  ASSERT_NE(&logger1, &logger2);
  ASSERT_NE(&logger1, &logger3);
  ASSERT_NE(&logger2, &logger3);

  logger1() << "test";
  logger2() << "other";
  ASSERT_FALSE(outbuf1.str().empty());
  ASSERT_FALSE(outbuf2.str().empty());
  ASSERT_EQ(logger1.name(), "anonymous_logger");
  ASSERT_EQ(logger2.name(), "anonymous_logger");
  ASSERT_EQ(logger3.name(), "anonymous_logger");
  ASSERT_NE(outbuf1.str(), outbuf2.str());
  ASSERT_TRUE(contains(outbuf1, "test"));
  ASSERT_TRUE(contains(outbuf2, "other"));
}

TEST(LoggerTest, namedLogger) {
  std::stringstream outbuf1;
  std::stringstream outbuf2;
  std::stringstream outbuf3;
  auto& logger1 = Logger::get("logger1", outbuf1);
  auto logger2 = Logger::get("logger2", outbuf2);
  auto& logger3 = Logger::get("logger1");
  auto logger4 = Logger::get("logger1", outbuf3);

  // created for SFINAE test
  auto logger5 = Logger::get(outbuf3, outbuf2);
  auto& logger6 = Logger::get("logger1", outbuf3, outbuf1);
  auto logger7 = Logger::get(outbuf3, outbuf2, outbuf1);
  auto& logger8 = Logger::get("logger1", outbuf3, outbuf1, outbuf1);
  auto logger9 = Logger::get(outbuf1);

  ASSERT_NE(&logger1, &logger2);
  ASSERT_EQ(&logger1, &logger3);
  ASSERT_NE(&logger1, &logger4);
  ASSERT_NE(&logger2, &logger4);

  logger1() << "one test";
  logger2() << "another test";
  logger3() << "yet one more";
  logger4() << "final test";

  ASSERT_TRUE(outbuf1.str().find(("one test")) != std::string::npos);
  ASSERT_FALSE(outbuf1.str().find(("another test")) != std::string::npos);
  ASSERT_TRUE(outbuf1.str().find(("yet one more")) != std::string::npos);
  ASSERT_TRUE(outbuf1.str().find(("final test")) != std::string::npos);

  ASSERT_FALSE(outbuf2.str().find(("one test")) != std::string::npos);
  ASSERT_TRUE(outbuf2.str().find(("another test")) != std::string::npos);
  ASSERT_FALSE(outbuf2.str().find(("yet one more")) != std::string::npos);
  ASSERT_FALSE(outbuf2.str().find(("final test")) != std::string::npos);
}

TEST(LoggerTest, timestampCoverage) {
  /// Pointless to compare timestamp correctly, since ms can shift it
  stringstream outbuf;
  auto logger = Logger::get(outbuf);

  logger.enableOptions(Logger::OptionFlag::Timestamp);
  logger() << "test";

  logger.enableOptions(Logger::OptionFlag::InfoPrefix);
  logger() << "test2";
}

TEST(LoggerTest, otherCoverage) {
  /// Other functions that just require coverage, do not make a difference
  auto l = Logger::get();
  l.enableOptions(Logger::OptionFlag::SourceLocation | Logger::OptionFlag::SourceLocationFunction);
  l.disableOptions(Logger::OptionFlag::SourceLocationFunction);
  auto const& l2 = Logger::get("test");
  auto const& l3 = Logger::get("test", cout, cout);
  (void) l2;
  (void) l3;
}
#else
TEST(LoggerTest, basicOut) {
  stringstream outbuf;
  auto logger = Logger::get(outbuf);
  logger() << "basic string output, followed by numeric: " << 123 << std::hex << 15 << std::dec << 24;
  ASSERT_TRUE(outbuf.str().empty());
}

#define WRAP_ASSERT_TRUE(...)                                                                                          \
  do {                                                                                                                 \
    auto v = __VA_ARGS__;                                                                                              \
    ASSERT_TRUE(v);                                                                                                    \
  } while (false)

TEST(LoggerTest, metainfo) {
  using O = Logger::OptionFlag;
  ASSERT_TRUE(flagTest());
  ASSERT_TRUE(flagTest<O::SourceLocation>());
  ASSERT_TRUE(flagTest<O::SourceLocationFile>());
  ASSERT_TRUE(flagTest<O::SourceLocationLine>());
  ASSERT_TRUE(flagTest<O::SourceLocationFunction>());
  ASSERT_TRUE(flagTest<O::LoggerName>());
  WRAP_ASSERT_TRUE(flagTest<O::InfoPrefix, O::LoggerName>());
  ASSERT_TRUE(flagTest<O::LogLevel>());
  WRAP_ASSERT_TRUE(flagTest<O::InfoPrefix, O::LogLevel>());
  ASSERT_TRUE(flagTest<O::ThreadId>());
  WRAP_ASSERT_TRUE(flagTest<O::InfoPrefix, O::ThreadId>());

  WRAP_ASSERT_TRUE(flagTest<O::SourceLocation, O::LoggerName>());
  WRAP_ASSERT_TRUE(flagTest<O::SourceLocation, O::LogLevel>());
  WRAP_ASSERT_TRUE(flagTest<O::SourceLocation, O::LoggerName, O::LogLevel>());

  WRAP_ASSERT_TRUE(flagTest<O::SourceLocation, O::InfoPrefix, O::LoggerName>());
  WRAP_ASSERT_TRUE(flagTest<O::SourceLocation, O::InfoPrefix, O::LogLevel>());
  WRAP_ASSERT_TRUE(flagTest<O::SourceLocation, O::InfoPrefix, O::LoggerName, O::LogLevel>());

  WRAP_ASSERT_TRUE(flagTest<O::SourceLocation, O::LoggerName, O::ThreadId>());
  WRAP_ASSERT_TRUE(flagTest<O::SourceLocation, O::LogLevel, O::ThreadId>());
  WRAP_ASSERT_TRUE(flagTest<O::SourceLocation, O::LoggerName, O::LogLevel, O::ThreadId>());

  WRAP_ASSERT_TRUE(flagTest<O::SourceLocation, O::InfoPrefix, O::LoggerName, O::ThreadId>());
  WRAP_ASSERT_TRUE(flagTest<O::SourceLocation, O::InfoPrefix, O::LogLevel, O::ThreadId>());
  WRAP_ASSERT_TRUE(flagTest<O::SourceLocation, O::InfoPrefix, O::LoggerName, O::LogLevel, O::ThreadId>());

  WRAP_ASSERT_TRUE(flagTest<O::OutputTerminalColour, O::SourceLocation, O::LoggerName>());
  WRAP_ASSERT_TRUE(flagTest<O::OutputTerminalColour, O::SourceLocation, O::LogLevel>());
  WRAP_ASSERT_TRUE(flagTest<O::OutputTerminalColour, O::SourceLocation, O::LoggerName, O::LogLevel>());

  WRAP_ASSERT_TRUE(flagTest<O::OutputTerminalColour, O::SourceLocation, O::InfoPrefix, O::LoggerName>());
  WRAP_ASSERT_TRUE(flagTest<O::OutputTerminalColour, O::SourceLocation, O::InfoPrefix, O::LogLevel>());
  WRAP_ASSERT_TRUE(flagTest<O::OutputTerminalColour, O::SourceLocation, O::InfoPrefix, O::LoggerName, O::LogLevel>());

  WRAP_ASSERT_TRUE(flagTest<O::OutputTerminalColour, O::SourceLocation, O::LoggerName, O::ThreadId>());
  WRAP_ASSERT_TRUE(flagTest<O::OutputTerminalColour, O::SourceLocation, O::LogLevel, O::ThreadId>());
  WRAP_ASSERT_TRUE(flagTest<O::OutputTerminalColour, O::SourceLocation, O::LoggerName, O::LogLevel, O::ThreadId>());

  WRAP_ASSERT_TRUE(flagTest<O::OutputTerminalColour, O::SourceLocation, O::InfoPrefix, O::LoggerName, O::ThreadId>());
  WRAP_ASSERT_TRUE(flagTest<O::OutputTerminalColour, O::SourceLocation, O::InfoPrefix, O::LogLevel, O::ThreadId>());
  WRAP_ASSERT_TRUE(
      flagTest<O::OutputTerminalColour, O::SourceLocation, O::InfoPrefix, O::LoggerName, O::LogLevel, O::ThreadId>());
}

TEST(LoggerTest, levelSwitch) {
  stringstream outbuf;
  auto logger = Logger::get(outbuf);

  logger.setOptions(Logger::OptionFlag::LogLevel);

  auto testImplicitLevel = [&outbuf, &logger](Logger::Level expectedLevel) {
    stringstream().swap(outbuf);
    logger() << "12345678";
    ASSERT_TRUE(outbuf.str().empty());
    ASSERT_FALSE(contains(outbuf, "12345678"));
    ASSERT_FALSE(contains(outbuf, "Info"));
    ASSERT_FALSE(contains(outbuf, "Warning"));
    ASSERT_FALSE(contains(outbuf, "Debug"));
    ASSERT_FALSE(contains(outbuf, "Error"));
  };

  auto testExplicitLevel = [&outbuf, &logger](Logger::Level level) {
    stringstream().swap(outbuf);
    logger(level) << "12345678";
    ASSERT_TRUE(outbuf.str().empty());
    ASSERT_FALSE(contains(outbuf, "12345678"));
    ASSERT_FALSE(contains(outbuf, "Info"));
    ASSERT_FALSE(contains(outbuf, "Warning"));
    ASSERT_FALSE(contains(outbuf, "Debug"));
    ASSERT_FALSE(contains(outbuf, "Error"));
  };

  auto testSwitch = [&testImplicitLevel, &testExplicitLevel](Logger::Level expectedImplicit) {
    testImplicitLevel(expectedImplicit);
    testExplicitLevel(Logger::Level::Warning);
    testImplicitLevel(expectedImplicit);
    testExplicitLevel(Logger::Level::Debug);
    testImplicitLevel(expectedImplicit);
    testExplicitLevel(Logger::Level::Error);
    testImplicitLevel(expectedImplicit);
    testExplicitLevel(Logger::Level::Info);
    testImplicitLevel(expectedImplicit);
  };

  testSwitch(Logger::Level::Info);
  logger.setDefaultLevel(Logger::Level::Warning);
  testSwitch(Logger::Level::Warning);
  logger.setDefaultLevel(Logger::Level::Error);
  testSwitch(Logger::Level::Error);
  logger.setDefaultLevel(Logger::Level::Debug);
  testSwitch(Logger::Level::Debug);
  logger.setDefaultLevel(Logger::Level::Info);
  testSwitch(Logger::Level::Info);
}

TEST(LoggerTest, defaultOut) {
  stringstream out;
  auto logger = Logger::get();
  logger.disableOptions(Logger::defaultOptionFlags);

  logger() << "<Text outputted part of test, to be ignored>";
  ASSERT_TRUE(out.str().empty());

  logger.enableOptions(Logger::OptionFlag::OutputTerminalColour);
  logger(Logger::Level::Info) << "<Text outputted part of test, to be ignored>";
  ASSERT_TRUE(out.str().empty());
  logger(Logger::Level::Warning) << "<Text outputted part of test, to be ignored>";
  ASSERT_TRUE(out.str().empty());
  logger(Logger::Level::Debug) << "<Text outputted part of test, to be ignored>";
  ASSERT_TRUE(out.str().empty());
  logger(Logger::Level::Error) << "<Text outputted part of test, to be ignored>";
  ASSERT_TRUE(out.str().empty());

  Logger::setDefaultOutput(out);
  auto newLogger = Logger::get();
  newLogger() << "<Text outputted part of test, to be ignored>";
  ASSERT_FALSE(contains(out, "<Text outputted part of test, to be ignored>"));
}

TEST(LoggerTest, endLining) {
  stringstream out;
  auto logger = Logger::get(out);

  std::cout << std::endl;
  logger() << "test" << std::endl << "another";
  ASSERT_TRUE(out.str().empty());
  ASSERT_FALSE(contains(out, "test\nanother"));
}

TEST(LoggerTest, individual) {
  auto [inf, wrn, err, dbg] = filterIndividual();

  ASSERT_FALSE(contains(inf, "Info"));
  ASSERT_FALSE(contains(inf, "test1"));
  ASSERT_FALSE(contains(inf, "Warning"));
  ASSERT_FALSE(contains(inf, "test2"));
  ASSERT_FALSE(contains(inf, "Error"));
  ASSERT_FALSE(contains(inf, "test3"));
  ASSERT_FALSE(contains(inf, "Debug"));
  ASSERT_FALSE(contains(inf, "test4"));

  ASSERT_FALSE(contains(wrn, "Info"));
  ASSERT_FALSE(contains(wrn, "test1"));
  ASSERT_FALSE(contains(wrn, "Warning"));
  ASSERT_FALSE(contains(wrn, "test2"));
  ASSERT_FALSE(contains(wrn, "Error"));
  ASSERT_FALSE(contains(wrn, "test3"));
  ASSERT_FALSE(contains(wrn, "Debug"));
  ASSERT_FALSE(contains(wrn, "test4"));

  ASSERT_FALSE(contains(err, "Info"));
  ASSERT_FALSE(contains(err, "test1"));
  ASSERT_FALSE(contains(err, "Warning"));
  ASSERT_FALSE(contains(err, "test2"));
  ASSERT_FALSE(contains(err, "Error"));
  ASSERT_FALSE(contains(err, "test3"));
  ASSERT_FALSE(contains(err, "Debug"));
  ASSERT_FALSE(contains(err, "test4"));

  ASSERT_FALSE(contains(dbg, "Info"));
  ASSERT_FALSE(contains(dbg, "test1"));
  ASSERT_FALSE(contains(dbg, "Warning"));
  ASSERT_FALSE(contains(dbg, "test2"));
  ASSERT_FALSE(contains(dbg, "Error"));
  ASSERT_FALSE(contains(dbg, "test3"));
  ASSERT_FALSE(contains(dbg, "Debug"));
  ASSERT_FALSE(contains(dbg, "test4"));
}

TEST(LoggerTest, mixedOut) {
  auto [infWrn, wrnErr, errDbg] = filterMixedOut();

  ASSERT_FALSE(contains(infWrn, "Info"));
  ASSERT_FALSE(contains(infWrn, "test1"));
  ASSERT_FALSE(contains(infWrn, "Warning"));
  ASSERT_FALSE(contains(infWrn, "test2"));
  ASSERT_FALSE(contains(infWrn, "Error"));
  ASSERT_FALSE(contains(infWrn, "test3"));
  ASSERT_FALSE(contains(infWrn, "Debug"));
  ASSERT_FALSE(contains(infWrn, "test4"));

  ASSERT_FALSE(contains(wrnErr, "Info"));
  ASSERT_FALSE(contains(wrnErr, "test1"));
  ASSERT_FALSE(contains(wrnErr, "Warning"));
  ASSERT_FALSE(contains(wrnErr, "test2"));
  ASSERT_FALSE(contains(wrnErr, "Error"));
  ASSERT_FALSE(contains(wrnErr, "test3"));
  ASSERT_FALSE(contains(wrnErr, "Debug"));
  ASSERT_FALSE(contains(wrnErr, "test4"));

  ASSERT_FALSE(contains(errDbg, "Info"));
  ASSERT_FALSE(contains(errDbg, "test1"));
  ASSERT_FALSE(contains(errDbg, "Warning"));
  ASSERT_FALSE(contains(errDbg, "test2"));
  ASSERT_FALSE(contains(errDbg, "Error"));
  ASSERT_FALSE(contains(errDbg, "test3"));
  ASSERT_FALSE(contains(errDbg, "Debug"));
  ASSERT_FALSE(contains(errDbg, "test4"));
}

TEST(LoggerTest, multiOutBasic) {
  stringstream out1;
  stringstream out2;
  auto logger = Logger::get(out1, out2);
  logger() << "basic string output, followed by numeric: " << 123 << std::hex << 15 << std::dec << 24;
  ASSERT_EQ(out1.str(), out2.str());
  ASSERT_FALSE(contains(out1.str(), "basic string output, followed by numeric: 123f24"));
  ASSERT_FALSE(contains(out2.str(), "basic string output, followed by numeric: 123f24"));
}

TEST(LoggerTest, isolation) {
  stringstream out1;
  stringstream out2;
  auto logger = Logger::get(out1, out2);

  logger() << "test";
  stringstream().swap(out2);
  ASSERT_TRUE(out1.str().empty());
  ASSERT_TRUE(out2.str().empty());
  ASSERT_FALSE(contains(out1, "test"));
  ASSERT_FALSE(contains(out2, "test"));
}

TEST(LoggerTest, constOutputs) {
  stringstream sb1;
  stringstream sb2;
  stringstream sb3;
  auto l = Logger::get();
  l.outputs() = {sb1, sb2};
  auto const& cl = l;
  ASSERT_EQ(cl.outputs().size(), 2);
  ASSERT_TRUE(cl.outputs().any([&sb1](auto const& l) { return &l.output() == &sb1; }));
  ASSERT_TRUE(cl.outputs().any([&sb2](auto const& l) { return &l.output() == &sb2; }));
  ASSERT_FALSE(cl.outputs().any([&sb3](auto const& l) { return &l.output() == &sb3; }));
}

TEST(LoggerTest, naming) {
  auto const& logger = Logger::get("testLog");
  ASSERT_EQ(logger.name(), "anonymous_logger");
}

TEST(LoggerTest, anonymousLogger) {
  stringstream outbuf1;
  auto logger1 = Logger::get(outbuf1);
  stringstream outbuf2;
  auto logger2 = Logger::get(outbuf2);
  auto logger3 = Logger::get();

  ASSERT_NE(&logger1, &logger2);
  ASSERT_NE(&logger1, &logger3);
  ASSERT_NE(&logger2, &logger3);

  logger1() << "test";
  logger2() << "other";
  ASSERT_TRUE(outbuf1.str().empty());
  ASSERT_TRUE(outbuf2.str().empty());
  ASSERT_EQ(logger1.name(), "anonymous_logger");
  ASSERT_EQ(logger2.name(), "anonymous_logger");
  ASSERT_EQ(logger3.name(), "anonymous_logger");
  ASSERT_EQ(outbuf1.str(), outbuf2.str());
  ASSERT_FALSE(contains(outbuf1, "test"));
  ASSERT_FALSE(contains(outbuf2, "other"));
}

TEST(LoggerTest, namedLogger) {
  std::stringstream outbuf1;
  std::stringstream outbuf2;
  std::stringstream outbuf3;
  auto& logger1 = Logger::get("logger1", outbuf1);
  auto logger2 = Logger::get("logger2", outbuf2);
  auto& logger3 = Logger::get("logger1");
  auto logger4 = Logger::get("logger1", outbuf3);

  // created for SFINAE test
  auto logger5 = Logger::get(outbuf3, outbuf2);
  auto& logger6 = Logger::get("logger1", outbuf3, outbuf1);
  auto logger7 = Logger::get(outbuf3, outbuf2, outbuf1);
  auto& logger8 = Logger::get("logger1", outbuf3, outbuf1, outbuf1);
  auto logger9 = Logger::get(outbuf1);

  ASSERT_NE(&logger1, &logger2);
  ASSERT_EQ(&logger1, &logger3);
  ASSERT_NE(&logger1, &logger4);
  ASSERT_NE(&logger2, &logger4);

  logger1() << "one test";
  logger2() << "another test";
  logger3() << "yet one more";
  logger4() << "final test";

  ASSERT_FALSE(outbuf1.str().find(("one test")) != std::string::npos);
  ASSERT_FALSE(outbuf1.str().find(("another test")) != std::string::npos);
  ASSERT_FALSE(outbuf1.str().find(("yet one more")) != std::string::npos);
  ASSERT_FALSE(outbuf1.str().find(("final test")) != std::string::npos);

  ASSERT_FALSE(outbuf2.str().find(("one test")) != std::string::npos);
  ASSERT_FALSE(outbuf2.str().find(("another test")) != std::string::npos);
  ASSERT_FALSE(outbuf2.str().find(("yet one more")) != std::string::npos);
  ASSERT_FALSE(outbuf2.str().find(("final test")) != std::string::npos);
}

TEST(LoggerTest, timestampCoverage) {
  /// Pointless to compare timestamp correctly, since ms can shift it
  stringstream outbuf;
  auto logger = Logger::get(outbuf);

  logger.enableOptions(Logger::OptionFlag::Timestamp);
  logger() << "test";

  logger.enableOptions(Logger::OptionFlag::InfoPrefix);
  logger() << "test2";
}

TEST(LoggerTest, otherCoverage) {
  /// Other functions that just require coverage, do not make a difference
  auto l = Logger::get();
  l.enableOptions(Logger::OptionFlag::SourceLocation | Logger::OptionFlag::SourceLocationFunction);
  l.disableOptions(Logger::OptionFlag::SourceLocationFunction);
  auto const& l2 = Logger::get("test");
  auto const& l3 = Logger::get("test", cout, cout);
  (void) l2;
  (void) l3;
}
#endif
