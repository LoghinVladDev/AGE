//
// Created by loghin on 8/21/23.
//

#pragma once

#include <CDS/Array>
#include <CDS/Tuple>
#include <CDS/memory/UniquePointer>
#include <CDS/threading/Mutex>

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

  explicit(false) LoggerOutput(std::ostream& out, FilterFlags filterFlags = allowAll) noexcept;

  LoggerOutput(std::ostream& out, FilterFlagBits filterLevel) noexcept :
      LoggerOutput(out, static_cast<FilterFlags>(filterLevel)) {}

  [[nodiscard]] auto outData() noexcept { return LockedOutput(_out); }

  [[nodiscard]] constexpr auto allows(meta::LogLevelFlags levels) const noexcept {
    assert((levels == (levels & mask)) && "Level requested outside valid Log Level values");
    return (_filter & levels) != 0u;
  }

  [[nodiscard]] constexpr auto allows(meta::LogLevelFlagBits level) const noexcept {
    return allows(static_cast<meta::LogLevelFlags>(level));
  }

private:
  using OutData = cds::Tuple<std::ostream*, cds::Mutex*>;

  class LockedOutput {
  public:
    explicit LockedOutput(OutData& out);
    ~LockedOutput();

    [[nodiscard]] constexpr auto& output() noexcept { return *_out.get<0>(); }

  private:
    OutData& _out;
  };

  OutData _out;
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
  using OptionFlag = LogOptionFlagBits;

  LoggerImplBase() noexcept = default;
  explicit LoggerImplBase(LoggerOutput out) noexcept : _outputs(1u, out) {}
  auto& outputs() noexcept { return _outputs; }
  [[nodiscard]] auto const& outputs() const noexcept { return _outputs; }

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

  [[nodiscard]] constexpr auto name() const noexcept {
    (void) this;
    return "anonymous_logger";
  }

  [[nodiscard]] constexpr auto defaultLevel() const noexcept {
    (void) this;
    return Level::Info;
  }

  [[nodiscard]] auto setDefaultLevel(Level level) const noexcept {
    (void) this;
    (void) level;
  }

  constexpr auto enableOptions(LogOptionFlags optionFlags) const noexcept -> void {
    (void) this;
    (void) optionFlags;
  }

  constexpr auto disableOptions(LogOptionFlags optionFlags) const noexcept -> void {
    (void) this;
    (void) optionFlags;
  }

  constexpr auto setOptions(LogOptionFlags optionFlags) const noexcept -> void {
    (void) this;
    (void) optionFlags;
  }

  constexpr auto enableOptions(LogOptionFlagBits optionFlag) const noexcept -> void {
    (void) this;
    (void) optionFlag;
  }

  constexpr auto disableOptions(LogOptionFlagBits optionFlag) const noexcept -> void {
    (void) this;
    (void) optionFlag;
  }

  constexpr auto setOptions(LogOptionFlagBits optionFlag) const noexcept -> void {
    (void) this;
    (void) optionFlag;
  }

  auto header(std::ostream& out, std::source_location const& where, Level level) const noexcept {
    (void) this;
    (void) out;
    (void) where;
    (void) level;
  }

  template <typename T> auto write(std::ostream& out, T&& data, Level level) const noexcept -> void {
    (void) this;
    (void) out;
    (void) data;
    (void) level;
  }

  auto modify(std::ostream& out, std::ostream& (*pfn)(std::ostream&), Level level) const noexcept -> void {
    (void) this;
    (void) out;
    (void) pfn;
    (void) level;
  }

  auto footer(std::string const& contents, Level level) const {
    (void) this;
    (void) contents;
    (void) level;
  }
};

template <> class LoggerImpl<cds::meta::BoolConstant<true>> : protected LoggerImplBase {
public:
  [[nodiscard]] constexpr auto const& name() const noexcept { return _name; }
  [[nodiscard]] constexpr auto defaultLevel() const noexcept { return _defaultLevel; }

  auto setDefaultLevel(Level level) noexcept { _defaultLevel = level; }

protected:
  LoggerImpl(StringRef name, LoggerOutput out) noexcept : LoggerImplBase(out), _name(name) {}

  auto header(std::ostream& out, std::source_location const& where, Level level) const { _header(out, where, level); }

  template <typename T> auto write(std::ostream& out, T&& data, Level level) const noexcept -> void {
    out << std::forward<T>(data);
  }

  auto modify(std::ostream& out, std::ostream& (*pfn)(std::ostream&), Level level) const noexcept -> void {
    out << pfn;
  }

  auto footer(std::string const& contents, Level level) { _footer(contents, level); }

  constexpr auto enableOptions(LogOptionFlags optionFlags) noexcept -> void {
    _options |= addRequirements(optionFlags & logOptionsMask);
  }

  constexpr auto disableOptions(LogOptionFlags optionFlags) noexcept -> void {
    _options &= ~(removeRequirements(optionFlags & logOptionsMask));
  }

  constexpr auto setOptions(LogOptionFlags optionFlags) noexcept -> void {
    _options = addRequirements(optionFlags & logOptionsMask);
  }

  constexpr auto enableOptions(LogOptionFlagBits optionFlag) noexcept -> void {
    _options |= addRequirements(optionFlag & logOptionsMask);
  }

  constexpr auto disableOptions(LogOptionFlagBits optionFlag) noexcept -> void {
    _options &= ~(removeRequirements(optionFlag & logOptionsMask));
  }

  constexpr auto setOptions(LogOptionFlagBits optionFlag) noexcept -> void {
    _options = addRequirements(optionFlag & logOptionsMask);
  }

private:
  auto _header(std::ostream& out, std::source_location const& where, Level level) const -> void;
  auto _footer(std::string const& contents, Level level) -> void;

  auto addColourHeader(std::ostream& out, Level level) const -> void;
  auto addEndColourMarker(std::ostream& out) const -> void;
  auto addLocation(std::ostream& out, std::source_location const& where) const -> void;
  auto addTimestamp(std::ostream& out, StringRef timestamp) const -> void;
  auto addName(std::ostream& out) const -> void;
  auto addLevel(std::ostream& out, Level level) const -> void;
  auto addThreadId(std::ostream& out) const -> void;
  auto addHeaderSpacing(std::ostream& out) const -> void;

  static constexpr auto addRequirements(LogLevelFlags flags) noexcept -> LogLevelFlags {
    using enum age::meta::LogOptionFlagBits;
    if ((flags & requireSourceLocation) != 0u) {
      flags |= SourceLocation;
    } else if ((flags & SourceLocation) != 0u) {
      flags |= SourceLocationFile | SourceLocationLine;
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
  using meta::LoggerImpl<>::Level;
  using meta::LoggerImpl<>::OptionFlag;

  auto operator()(std::source_location const& location = std::source_location::current()) noexcept {
    return LogWriter {this, defaultLevel(), location};
  }

  auto operator()(Level level, std::source_location const& location = std::source_location::current()) noexcept {
    return LogWriter {this, level, location};
  }

  template <typename... Outputs>
    requires(sizeof...(Outputs) > 1
             && cds::meta::All<cds::meta::Bind<cds::meta::IsConvertible, cds::meta::Ph<1>, LoggerOutput>::Type,
                               Outputs...>::value)
  static auto get(StringRef name, Outputs&&... outputs) noexcept -> Logger& {
    auto& logger = get(name);
    auto& outArr = logger.outputs();

    if (outArr.size() == 1u && &outArr[0u].outData().output() == &defaultOutput()) {
      outArr.clear();
    }

    outArr.insertAll(std::forward<Outputs>(outputs)...);
    return logger;
  }

  template <typename... Outputs>
    requires(sizeof...(Outputs) > 1
             && cds::meta::All<cds::meta::Bind<cds::meta::IsConvertible, cds::meta::Ph<1>, LoggerOutput>::Type,
                               Outputs...>::value)
  static auto get(Outputs&&... outputs) noexcept -> Logger {
    auto logger = get();
    auto& outArr = logger.outputs();
    outArr.clear();
    outArr.insertAll(std::forward<Outputs>(outputs)...);
    return logger;
  }

  static auto get(StringRef name, std::ostream& out) noexcept -> Logger&;
  static auto get(StringRef name) noexcept -> Logger&;
  static auto get(std::ostream& out) noexcept -> Logger;
  static auto get() noexcept -> Logger;
  static auto setDefaultOutput(std::ostream& out) noexcept -> void;
  static auto defaultOutput() noexcept -> std::ostream&;

  using meta::LoggerImpl<>::defaultOptionFlags;

  using meta::LoggerImpl<>::name;
  using meta::LoggerImpl<>::outputs;
  using meta::LoggerImpl<>::enableOptions;
  using meta::LoggerImpl<>::disableOptions;
  using meta::LoggerImpl<>::setOptions;
  using meta::LoggerImpl<>::setDefaultLevel;

private:
  class LogWriter {
  public:
    LogWriter(Logger* pLogger, Level level, std::source_location const& location) : _pLogger(pLogger), _level(level) {
      _pLogger->header(_localBuffer, location, level);
    }

    template <typename T> auto& operator<<(T&& data) {
      _pLogger->write(_localBuffer, std::forward<T>(data), _level);
      return *this;
    }

    auto& operator<<(std::ostream& (*pfn)(std::ostream&) ) {
      _pLogger->modify(_localBuffer, pfn, _level);
      return *this;
    }

    ~LogWriter() noexcept { _pLogger->footer(_localBuffer.str(), _level); }

  private:
    Logger* _pLogger;
    Level _level;
    std::stringstream _localBuffer;
  };

  using meta::LoggerImpl<>::LoggerImpl;
};
} // namespace age
