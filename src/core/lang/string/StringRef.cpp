//
// Created by Vlad-Andrei Loghin on 26.06.23.
//

#include "StringRef.hpp"
#include <cassert>
#include <exception>

using namespace age;
using namespace cds;

namespace {
auto toWeakOrdering(sint8 value) noexcept -> std::weak_ordering {
  switch (value) {
    case StringUtils<char>::lesser: return std::weak_ordering::less;
    case StringUtils<char>::greater: return std::weak_ordering::greater;
    case StringUtils<char>::equal: return std::weak_ordering::equivalent;
  }

  assert(false && "Uncovered StringUtils<char>::equality_type");
  std::terminate();
}
} // namespace

StringRef::StringRef(String const& string) noexcept : StringRef(string.cStr(), string.size()) {}
StringRef::StringRef(StringView const& string) noexcept : StringRef(string.cStr(), string.size()) {}
StringRef::StringRef(std::string const& string) noexcept : StringRef(string.c_str(), string.size()) {}
StringRef::StringRef(std::string_view const& string) noexcept : StringRef(string.data(), string.length()) {}
StringRef::StringRef(char const* string) noexcept : StringRef(string, Utils::length(string)) {}
StringRef::StringRef(char const* string, cds::Size length) noexcept : _buffer(string), _size(length) {}

auto StringRef::operator=(String const& string) noexcept -> StringRef& {
  _buffer = string.cStr();
  _size = string.size();
  return *this;
}

auto StringRef::operator=(StringView const& string) noexcept -> StringRef& {
  _buffer = string.cStr();
  _size = string.size();
  return *this;
}

auto StringRef::operator=(std::string const& string) noexcept -> StringRef& {
  _buffer = string.c_str();
  _size = string.size();
  return *this;
}

auto StringRef::operator=(std::string_view const& string) noexcept -> StringRef& {
  _buffer = string.data();
  _size = string.size();
  return *this;
}

auto StringRef::operator=(char const* string) noexcept -> StringRef& {
  _buffer = string;
  _size = Utils::length(string);
  return *this;
}

StringRef::operator bool() const noexcept { return !empty(); }
StringRef::operator StringView() const noexcept { return {data(), size()}; }
StringRef::operator String() const noexcept { return {data(), size()}; }

auto StringRef::dropFront(Size amount) const noexcept -> StringRef {
  if (amount >= size()) {
    return {};
  }
  return {data() + amount, size() - amount};
}

auto StringRef::dropBack(Size amount) const noexcept -> StringRef {
  if (amount >= size()) {
    return {};
  }
  return {data(), size() - amount};
}

auto StringRef::takeFront(cds::Size amount) const noexcept -> StringRef {
  if (amount >= size()) {
    return *this;
  }

  return {data(), amount};
}

auto StringRef::takeBack(cds::Size amount) const noexcept -> StringRef {
  if (amount >= size()) {
    return *this;
  }

  return {data() + size() - amount, amount};
}

auto StringRef::shrink(Size amount) noexcept -> StringRef& {
  if (amount >= size()) {
    clear();
    return *this;
  }
  _size = size() - amount;
  return *this;
}

auto StringRef::clear() noexcept -> void {
  _buffer = nullptr;
  _size = 0u;
}

auto StringRef::find(char character) const noexcept -> Index {
  auto buffer = data();
  for (Size index = 0; index < size(); ++index) {
    if (buffer[index] == character) {
      return static_cast<Index>(index);
    }
  }
  return npos;
}

auto StringRef::sub(Size offset, Size length) const noexcept -> StringRef {
  return dropFront(offset).takeFront(length);
}

auto StringRef::sub(Size offset) const noexcept -> StringRef { return dropFront(offset); }

auto StringRef::operator+(StringRef const& ref) const noexcept -> String {
  String result;
  result.resize(size() + ref.size() + 1);
  return std::move(result) + StringView(data(), size()) + StringView(ref.data(), ref.size());
}

auto StringRef::operator<=>(StringRef const& ref) const noexcept -> std::weak_ordering {
  return toWeakOrdering(StringUtils<char>::compare(data(), size(), ref.data(), ref.size()));
}
