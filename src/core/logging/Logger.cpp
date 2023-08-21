//
// Created by loghin on 8/21/23.
//

#include "Logger.hpp"

#include <format>

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
  [[nodiscard]] auto& defaultOut() const noexcept {
    (void) this;
    return cout;
  }

  auto setDefaultOut(ostream const& out) const noexcept {
    (void) this;
    (void) out;
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

auto timestamp() -> String { return "<time>"; }

auto toString(LogLevelFlagBit level) {
  switch (level) {
    using enum age::meta::LogLevelFlagBit;
    case Info: return "Info";
    case Debug: return "Debug";
    case Error: return "Error";
    case Warning: return "Warning";
  }
}

auto colour(LogLevelFlagBit level) {
  switch (level) {
    using enum age::meta::LogLevelFlagBit;
    case Error: return "\033[1;31m";
    case Warning: return "\033[1;33m";
    case Debug: return "\033[1;36m";
    case Info: return "\033[1;37m";
  }
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
  // Check if enabled
  (void) this;
  out << colour(level);
}

auto LoggerImpl<BoolConstant<true>>::addEndColourMarker(ostream& out) const -> void {
  // Check if enabled
  (void) this;
  out << "\033[1;0m";
}

auto LoggerImpl<BoolConstant<true>>::addLocation(ostream& out, source_location const& where) const -> void {
  // Check if enabled
  // Format source_location by pref
  (void) this;
  out << "[" << where.function_name() << ":" << where.line() << "]";
}

auto LoggerImpl<BoolConstant<true>>::addTimestamp(ostream& out, StringRef timestamp) const -> void {
  // Check if enabled
  (void) this;
  out << "[time = " << timestamp << "]";
}

auto LoggerImpl<BoolConstant<true>>::addName(ostream& out) const -> void {
  // Check if enabled
  out << "[logger = " << name() << "]";
}

auto LoggerImpl<BoolConstant<true>>::addLevel(ostream& out, Level level) const -> void {
  // Check if enabled
  (void) this;
  out << "[level = " << toString(level) << "]";
}

auto LoggerImpl<BoolConstant<true>>::addThreadId(ostream& out) const -> void {
  // Check if enabled
  (void) this;
  out << std::format("[thread = 0x{:x}]", Thread::currentThreadID());
}
} // namespace meta

auto Logger::get() noexcept -> Logger { return Logger {"anonymous_logger", container().defaultOut()}; }

auto Logger::get(age::StringRef name) noexcept -> Logger& {
  static auto whenDisabled = get();
  return container().get(Logger {name, container().defaultOut()}, whenDisabled);
}

auto Logger::setDefaultOutput(ostream& out) noexcept -> void { container().setDefaultOut(out); }
} // namespace age
