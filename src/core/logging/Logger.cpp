//
// Created by loghin on 8/21/23.
//

#include "Logger.hpp"

#include <chrono>

#define CI_FORMAT_AVAILABLE false

#if defined(__cpp_lib_format) && __cpp_lib_format > 202207l && CI_FORMAT_AVAILABLE
#include <format>
#endif

#include <CDS/TreeMap>
#include <CDS/threading/Thread>

namespace {
using namespace age;
using namespace age::meta;
using namespace cds;
using namespace cds::meta;
using namespace std;

template <typename = LoggingEnabled> class LoggerContainer {};

template <> class LoggerContainer<BoolConstant<false>> {
public:
  [[nodiscard]] [[maybe_unused]] auto& defaultOut() const noexcept {
    (void) this;
    return cout;
  }

  [[maybe_unused]] auto setDefaultOut(ostream const& out) const noexcept {
    (void) this;
    (void) out;
  }

  [[maybe_unused]] auto get(Logger const& hint, Logger& whenDisabled) noexcept -> Logger& {
    (void) hint;
    return whenDisabled;
  }
};

template <> class LoggerContainer<BoolConstant<true>> {
public:
  [[nodiscard]] auto& defaultOut() noexcept { return *_pDefaultOut; }
  auto setDefaultOut(ostream& out) noexcept { _pDefaultOut = &out; }

  auto get(Logger&& hint, Logger const& whenDisabled) noexcept -> Logger& {
    (void) whenDisabled;
    return _loggers.emplace(hint.name(), std::move(hint)).value();
  }

private:
  ostream* _pDefaultOut {&cout};
  TreeMap<StringRef, Logger> _loggers;
};

auto& container() noexcept {
  using C = LoggerContainer<>;
  static UniquePointer<C> container;
  if (!container) {
    container = makeUnique<C>();
  }
  return *container;
}

auto timestamp() {
#if defined(__cpp_lib_format) && __cpp_lib_format > 202207l && CI_FORMAT_AVAILABLE
  using namespace chrono;
  return std::format("{:%H:%M:%OS}", current_zone()->to_local(system_clock::now()));
#else
  int const timeBufferSize = 512U;
  using sys_clock = std::chrono::system_clock;

  auto timePoint = sys_clock::now();
  auto asTimeT = sys_clock::to_time_t(timePoint);
  tm timeInfo {};
  localtime_r(&asTimeT, &timeInfo);

  string asString(timeBufferSize, '\0');
  asString.resize(std::strftime(asString.data(), timeBufferSize, "%H:%M:%OS", &timeInfo));
  asString.resize(asString.length());

  return asString;
#endif
}

auto toString(LogLevelFlagBits level) {
  switch (level) {
    using enum age::meta::LogLevelFlagBits;
    case Info: return "Info";
    case Debug: return "Debug";
    case Error: return "Error";
    case Warning: return "Warning";
  }
}

auto colour(LogLevelFlagBits level) {
  switch (level) {
    using enum age::meta::LogLevelFlagBits;
    case Error: return "\033[1;31m";
    case Warning: return "\033[1;33m";
    case Debug: return "\033[1;36m";
    case Info: return "\033[1;37m";
  }
}

auto colourCompatibleOutput(std::ostream const& out) {
#if defined(__linux) | defined(__APPLE__)
  return (&out == &cout) || (&out == &clog) && isatty(1);
#else
  return false;
#endif
}
} // namespace

namespace age {
namespace meta {
auto LoggerImpl<BoolConstant<true>>::_header(source_location const& where, Level level) -> void {
  auto time = timestamp();
  for (auto& output : outputs()) {
    if (output.allows(level)) {
      auto& out = output.output();
      addColourHeader(out, level);
      addLocation(out, where);
      addTimestamp(out, time);
      addName(out);
      addLevel(out, level);
      addThreadId(out);
      addHeaderSpacing(out);
    }
  }
}

auto LoggerImpl<BoolConstant<true>>::_footer(Level level) -> void {
  for (auto& output : outputs()) {
    if (output.allows(level)) {
      auto& out = output.output();
      addEndColourMarker(out);
      out << endl;
    }
  }
}

auto LoggerImpl<BoolConstant<true>>::addColourHeader(ostream& out, Level level) const -> void {
  if (!colourCompatibleOutput(out)) {
    return;
  }
  if (!optionEnabled(LogOptionFlagBits::OutputTerminalColour)) {
    return;
  }
  out << colour(level);
}

auto LoggerImpl<BoolConstant<true>>::addEndColourMarker(ostream& out) const -> void {
  if (!colourCompatibleOutput(out)) {
    return;
  }
  if (!optionEnabled(LogOptionFlagBits::OutputTerminalColour)) {
    return;
  }
  out << "\033[1;0m";
}

auto LoggerImpl<BoolConstant<true>>::addLocation(ostream& out, source_location const& where) const -> void {
  if (!optionEnabled(LogOptionFlagBits::SourceLocation) || (_options & requireSourceLocation) == 0u) {
    return;
  }

  struct {
    auto request() noexcept { toggle = true; }
    auto consume() noexcept {
      if (!toggle) {
        return;
      }
      toggle = false;
      out << ":";
    }
    bool toggle;
    ostream& out;
  } sep {false, out};

  auto writeLocationPart = [this, &sep, &out](auto flag, auto data) {
    if (!optionEnabled(flag)) {
      return;
    }
    sep.consume();
    out << data;
    sep.request();
  };

  out << "[";
  writeLocationPart(LogOptionFlagBits::SourceLocationFile, where.file_name());
  writeLocationPart(LogOptionFlagBits::SourceLocationFunction, where.function_name());
  writeLocationPart(LogOptionFlagBits::SourceLocationLine, where.line());
  writeLocationPart(LogOptionFlagBits::SourceLocationColumn, where.column());
  out << "]";
}

auto LoggerImpl<BoolConstant<true>>::addTimestamp(ostream& out, StringRef timestamp) const -> void {
  if (!optionEnabled(LogOptionFlagBits::Timestamp)) {
    return;
  }
  out << "[";
  if (optionEnabled(LogOptionFlagBits::InfoPrefix)) {
    out << "time = ";
  }
  out << timestamp << "]";
}

auto LoggerImpl<BoolConstant<true>>::addName(ostream& out) const -> void {
  if (!optionEnabled(LogOptionFlagBits::LoggerName)) {
    return;
  }
  out << "[";
  if (optionEnabled(LogOptionFlagBits::InfoPrefix)) {
    out << "logger = ";
  }
  out << name() << "]";
}

auto LoggerImpl<BoolConstant<true>>::addLevel(ostream& out, Level level) const -> void {
  if (!optionEnabled(LogOptionFlagBits::LogLevel)) {
    return;
  }
  out << "[";
  if (optionEnabled(LogOptionFlagBits::InfoPrefix)) {
    out << "level = ";
  }
  out << toString(level) << "]";
}

auto LoggerImpl<BoolConstant<true>>::addThreadId(ostream& out) const -> void {
  if (!optionEnabled(LogOptionFlagBits::ThreadId)) {
    return;
  }
  out << "[";
  if (optionEnabled(LogOptionFlagBits::InfoPrefix)) {
    out << "thread = ";
  }
#if CI_FORMAT_AVAILABLE
  out << std::format("0x{:x}]", Thread::currentThreadID());
#else
  out << "0x" << std::hex << Thread::currentThreadID() << std::dec << "]";
#endif
}

auto LoggerImpl<BoolConstant<true>>::addHeaderSpacing(std::ostream& out) const -> void {
  using enum age::meta::LogOptionFlagBits;

  if (constexpr auto const visibleOptionsMask = InfoPrefix | SourceLocation | SourceLocationFile
          | SourceLocationFunction | SourceLocationLine | SourceLocationColumn | Timestamp | LoggerName | LogLevel
          | ThreadId;
      (visibleOptionsMask & _options) == 0u) {
    return;
  }
  out << " ";
}
} // namespace meta

auto Logger::get() noexcept -> Logger { return Logger {"anonymous_logger", container().defaultOut()}; }

auto Logger::get(StringRef name) noexcept -> Logger& {
  static auto whenDisabled = get();
  return container().get(Logger {name, container().defaultOut()}, whenDisabled);
}

auto Logger::get(ostream& out) noexcept -> Logger {
  auto logger = get();
  auto& outArr = logger.outputs();
  outArr.clear();
  outArr.emplace(out);
  return logger;
}

auto Logger::get(StringRef name, ostream& out) noexcept -> Logger& {
  auto& logger = get(name);
  auto& outArr = logger.outputs();

  if (outArr.size() == 1u && &outArr[0u].output() == &defaultOutput()) {
    outArr.clear();
  }

  outArr.emplace(out);
  return logger;
}

auto Logger::setDefaultOutput(ostream& out) noexcept -> void { container().setDefaultOut(out); }
auto Logger::defaultOutput() noexcept -> ostream& { return container().defaultOut(); }
} // namespace age
