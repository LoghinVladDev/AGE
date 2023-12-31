//
// Created by Vlad-Andrei Loghin on 07.07.23.
//

#pragma once
#include <CDS/Object>
#include <fstream>

namespace age {
namespace meta {
class PathAwareDirectoryCreator {
protected:
  explicit PathAwareDirectoryCreator(cds::StringView path) noexcept(false);
};

template <typename Base> class IfstreamFunctions {
public:
  template <typename T> auto operator>>(T&& object) noexcept -> auto& {
    static_cast<Base*>(this)->handle() << std::forward<T>(object);
    return *this;
  }

  [[nodiscard]] auto rdbuf() { return static_cast<Base*>(this)->handle().rdbuf(); }
};

template <typename Base> class OfstreamFunctions {
public:
  template <typename T> auto operator<<(T&& object) noexcept -> auto& {
    static_cast<Base*>(this)->handle() << std::forward<T>(object);
    return *this;
  }

  auto write(char const* buffer, std::size_t size) { static_cast<Base*>(this)->handle().write(buffer, size); }
};

template <typename Base> class FstreamFunctions {
public:
  auto close() { static_cast<Base*>(this)->handle().close(); }
};
} // namespace meta

class PathAwareFstream :
    public meta::PathAwareDirectoryCreator,
    public meta::FstreamFunctions<PathAwareFstream>,
    public meta::IfstreamFunctions<PathAwareFstream>,
    public meta::OfstreamFunctions<PathAwareFstream> {
public:
  explicit PathAwareFstream(cds::StringView path,
                            std::ios::openmode mode = std::ios::in | std::ios::out) noexcept(false) :
      meta::PathAwareDirectoryCreator(path),
      file(path.cStr(), mode) {}

  [[nodiscard]] constexpr auto handle() noexcept -> auto& { return file; }

private:
  std::fstream file;
};

class PathAwareOfstream :
    public meta::PathAwareDirectoryCreator,
    public meta::FstreamFunctions<PathAwareOfstream>,
    public meta::OfstreamFunctions<PathAwareOfstream> {
public:
  explicit PathAwareOfstream(cds::StringView path, std::ios::openmode mode = std::ios::out) noexcept(false) :
      meta::PathAwareDirectoryCreator(path), file(path.cStr(), mode) {}

  [[nodiscard]] constexpr auto handle() noexcept -> auto& { return file; }

private:
  std::ofstream file;
};
} // namespace age
