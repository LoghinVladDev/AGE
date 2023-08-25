//
// Created by loghin on 8/21/23.
//

#pragma once

#include <CDS/Array>
#include <CDS/memory/UniquePointer>

#include <source_location>

#include <lang/flag/FlagEnum.hpp>
#include <lang/string/StringRef.hpp>

namespace age {
class Logger;

namespace meta {
enum class LogLevelFlagBits : cds::uint32 { Info = 1u << 0, Debug = 1u << 1, Error = 1u << 2, Warning = 1u << 3 };
enum class LogOptionFlagBits : cds::uint32 {
  OutputTerminalColour = 1u << 0,
  InfoPrefix = 1u << 1,
  SourceLocation = 1u << 8,
  SourceLocationFile = 1u << 9,
  SourceLocationFunction = 1u << 10,
  SourceLocationLine = 1u << 11,
  SourceLocationColumn = 1u << 12,
  Timestamp = 1u << 16,
  LoggerName = 1u << 24,
  LogLevel = 1u << 25,
  ThreadId = 1u << 26,
};

using LogLevelFlags = std::underlying_type_t<LogLevelFlagBits>;
using LogOptionFlags = std::underlying_type_t<LogOptionFlagBits>;

template <> struct FlagEnum<LogLevelFlagBits> : cds::meta::TrueType {
  using FlagsType = LogLevelFlags;
};

template <> struct FlagEnum<LogOptionFlagBits> : cds::meta::TrueType {
  using FlagsType = LogOptionFlags;
};

static constexpr auto const logLevelMask =
    LogLevelFlagBits::Info | LogLevelFlagBits::Debug | LogLevelFlagBits::Warning | LogLevelFlagBits::Error;

static constexpr auto const logOptionsMask = LogOptionFlagBits::OutputTerminalColour | LogOptionFlagBits::InfoPrefix
    | LogOptionFlagBits::SourceLocation | LogOptionFlagBits::SourceLocationFile
    | LogOptionFlagBits::SourceLocationFunction | LogOptionFlagBits::SourceLocationLine
    | LogOptionFlagBits::SourceLocationColumn | LogOptionFlagBits::Timestamp | LogOptionFlagBits::LoggerName
    | LogOptionFlagBits::LogLevel | LogOptionFlagBits::ThreadId;
} // namespace meta

class LoggerOutput {
public:
  static constexpr auto const allowInfo = meta::LogLevelFlagBits::Info;
  static constexpr auto const allowWarning = meta::LogLevelFlagBits::Warning;
  static constexpr auto const allowDebug = meta::LogLevelFlagBits::Debug;
  static constexpr auto const allowError = meta::LogLevelFlagBits::Error;
  static constexpr auto const allowAll = allowInfo | allowWarning | allowDebug | allowError;

  using FilterFlags = meta::LogLevelFlags;
  using FilterFlagBits = meta::LogLevelFlagBits;

  explicit(false) LoggerOutput(std::ostream& out, FilterFlags filterFlags = allowAll) noexcept :
      _pOutput(&out), _filter(filterFlags & mask) {}

  LoggerOutput(std::ostream& out, FilterFlagBits filterLevel) noexcept :
      _pOutput(&out), _filter(static_cast<FilterFlags>(filterLevel)) {}

  [[nodiscard]] constexpr auto& output() noexcept { return *_pOutput; }
  [[nodiscard]] constexpr auto const& output() const noexcept { return *_pOutput; }

  [[nodiscard]] constexpr auto allows(meta::LogLevelFlags levels) const noexcept {
    return (_filter & levels & mask) != 0u;
  }

  [[nodiscard]] constexpr auto allows(meta::LogLevelFlagBits level) const noexcept {
    return allows(static_cast<meta::LogLevelFlags>(level));
  }

private:
  std::ostream* _pOutput;
  FilterFlags _filter;
  static constexpr auto const mask = meta::logLevelMask;
};

namespace meta {
#ifdef NDEBUG
using LoggingEnabled = cds::meta::BoolConstant<false>;
#else
using LoggingEnabled = cds::meta::BoolConstant<true>;
#endif

class LoggerImplBase {
public:
  static constexpr auto const defaultOptionFlags = LogOptionFlagBits::OutputTerminalColour
      | LogOptionFlagBits::SourceLocation | LogOptionFlagBits::SourceLocationFile
      | LogOptionFlagBits::SourceLocationLine | LogOptionFlagBits::Timestamp | LogOptionFlagBits::LoggerName
      | LogOptionFlagBits::LogLevel | LogOptionFlagBits::ThreadId;

protected:
  using Level = LogLevelFlagBits;
  using OptionFlag = LogLevelFlagBits;

  LoggerImplBase() noexcept = default;
  explicit LoggerImplBase(LoggerOutput out) noexcept : _outputs(1u, out) {}
  auto& outputs() noexcept { return _outputs; }

private:
  cds::Array<LoggerOutput> _outputs;
};

template <typename = LoggingEnabled> class LoggerImpl {};

template <> class LoggerImpl<cds::meta::BoolConstant<false>> : protected LoggerImplBase {
protected:
  LoggerImpl(StringRef name, LoggerOutput out) noexcept : LoggerImplBase() {
    (void) out;
    (void) name;
  }
};

template <> class LoggerImpl<cds::meta::BoolConstant<true>> : protected LoggerImplBase {
public:
  [[nodiscard]] constexpr auto const& name() const noexcept { return _name; }
  [[nodiscard]] constexpr auto defaultLevel() const noexcept { return _defaultLevel; }

  auto setDefaultLevel(Level level) noexcept { _defaultLevel = level; }

protected:
  LoggerImpl(StringRef name, LoggerOutput out) noexcept : LoggerImplBase(out), _name(name) {}

  auto header(std::source_location const& where, Level level) { _header(where, level); }

  template <typename T> auto write(T&& data, Level level) noexcept -> void {
    for (auto& output : outputs()) {
      if (output.allows(level)) {
        output.output() << std::forward<T>(data);
      }
    }
  }

  auto footer(Level level) { _footer(level); }

  constexpr auto enableOptions(LogLevelFlags optionFlags) noexcept -> void {
    _options |= addRequirements(optionFlags & logOptionsMask);
  }

  constexpr auto disableOptions(LogLevelFlags optionFlags) noexcept -> void {
    _options &= ~(removeRequirements(optionFlags & logOptionsMask));
  }

  constexpr auto setOptions(LogLevelFlags optionFlags) noexcept -> void {
    _options = addRequirements(optionFlags & logOptionsMask);
  }

private:
  auto _header(std::source_location const& where, Level level) -> void;
  auto _footer(Level level) -> void;

  auto addColourHeader(std::ostream& out, Level level) const -> void;
  auto addEndColourMarker(std::ostream& out) const -> void;
  auto addLocation(std::ostream& out, std::source_location const& where) const -> void;
  auto addTimestamp(std::ostream& out, StringRef timestamp) const -> void;
  auto addName(std::ostream& out) const -> void;
  auto addLevel(std::ostream& out, Level level) const -> void;
  auto addThreadId(std::ostream& out) const -> void;

  static constexpr auto addRequirements(LogLevelFlags flags) noexcept -> LogLevelFlags {
    if ((flags & requireSourceLocation) != 0u) {
      flags |= LogOptionFlagBits::SourceLocation;
    }
    return flags;
  }

  static constexpr auto removeRequirements(LogLevelFlags flags) noexcept -> LogLevelFlags {
    if ((flags & LogOptionFlagBits::SourceLocation) == 0u) {
      flags &= ~requireSourceLocation;
    }
    return flags;
  }

  [[nodiscard]] constexpr auto optionEnabled(LogOptionFlagBits bit) const noexcept {
    return (_options & (bit & logOptionsMask)) != 0u;
  }

  cds::String _name;
  Level _defaultLevel = Level::Info;
  LogOptionFlags _options = defaultOptionFlags;

  static constexpr auto const requireSourceLocation = LogOptionFlagBits::SourceLocationFile
      | LogOptionFlagBits::SourceLocationFunction | LogOptionFlagBits::SourceLocationLine
      | LogOptionFlagBits::SourceLocationColumn;
};
} // namespace meta

class Logger : protected meta::LoggerImpl<> {
public:
  auto operator()(std::source_location const& location = std::source_location::current()) noexcept {
    return LogWriter {this, defaultLevel(), location};
  }

  template <typename... Outputs>
    requires(sizeof...(Outputs) > 0
             && cds::meta::All<cds::meta::Bind<cds::meta::IsConvertible, cds::meta::Ph<1>, LoggerOutput>::Type,
                               Outputs...>::value)
  static auto get(StringRef name, Outputs&&... outputs) noexcept -> Logger {
    auto logger = get(name);
    auto& outArr = logger.outputs();
    outArr.clear();
    outArr.insertAllOf(std::forward<Outputs>(outputs)...);
    return logger;
  }

  template <typename... Outputs>
    requires(sizeof...(Outputs) > 0
             && cds::meta::All<cds::meta::Bind<cds::meta::IsConvertible, cds::meta::Ph<1>, LoggerOutput>::Type,
                               Outputs...>::value)
  static auto get(Outputs&&... outputs) noexcept -> Logger {
    auto logger = get();
    auto& outArr = logger.outputs();
    outArr.clear();
    outArr.insertAll(std::forward<Outputs>(outputs)...);
    return logger;
  }

  static auto get(StringRef name) noexcept -> Logger&;
  static auto get() noexcept -> Logger;
  static auto setDefaultOutput(std::ostream& out) noexcept -> void;

  using meta::LoggerImpl<>::name;

private:
  class LogWriter {
  public:
    LogWriter(Logger* pLogger, Level level, std::source_location const& location) : _pLogger(pLogger), _level(level) {
      _pLogger->header(location, level);
    }

    template <typename T> auto& operator<<(T&& data) {
      _pLogger->write(std::forward<T>(data), _level);
      return *this;
    }

    ~LogWriter() noexcept { _pLogger->footer(_level); }

  private:
    Logger* _pLogger;
    Level _level;
  };

  using meta::LoggerImpl<>::LoggerImpl;
  using meta::LoggerImpl<>::enableOptions;
  using meta::LoggerImpl<>::disableOptions;
  using meta::LoggerImpl<>::setOptions;
};
} // namespace age
