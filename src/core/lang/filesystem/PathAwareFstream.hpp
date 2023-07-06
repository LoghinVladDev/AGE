//
// Created by Vlad-Andrei Loghin on 07.07.23.
//

#ifndef AGE_PATH_AWARE_FSTREAM_HPP
#define AGE_PATH_AWARE_FSTREAM_HPP

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
};

template <typename Base> class OfstreamFunctions {
public:
  template <typename T> auto operator<<(T&& object) noexcept -> auto& {
    static_cast<Base*>(this)->handle() << std::forward<T>(object);
    return *this;
  }
};

template <typename Base> class FstreamFunctions {
  // To be filled later with common functions where needed.
};
} // namespace meta

class PathAwareFstream :
    public meta::PathAwareDirectoryCreator,
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

class PathAwareOfstream : public meta::PathAwareDirectoryCreator, public meta::OfstreamFunctions<PathAwareOfstream> {
public:
  explicit PathAwareOfstream(cds::StringView path, std::ios::openmode mode = std::ios::out) noexcept(false) :
      meta::PathAwareDirectoryCreator(path), file(path.cStr(), mode) {}

  [[nodiscard]] constexpr auto handle() noexcept -> auto& { return file; }

private:
  std::ofstream file;
};

class PathAwareIfstream : public meta::PathAwareDirectoryCreator, public meta::IfstreamFunctions<PathAwareIfstream> {
public:
  explicit PathAwareIfstream(cds::StringView path, std::ios::openmode mode = std::ios::in) noexcept(false) :
      meta::PathAwareDirectoryCreator(path), file(path.cStr(), mode) {}

  [[nodiscard]] constexpr auto handle() noexcept -> auto& { return file; }

private:
  std::ifstream file;
};
} // namespace age

#endif // AGE_PATH_AWARE_FSTREAM_HPP
