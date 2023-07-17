//
// Created by Vlad-Andrei Loghin on 12.07.23.
//

#pragma once
#include <vector>
#include <array>

#include <CDS/Array>
#include <CDS/StaticArray>
#include <lang/generic/Concepts.hpp>
#include <lang/generic/OperatorsWithImplicitConstruction.hpp>

namespace age {
template <typename T>
class ArrayRef {
public:
  ArrayRef() noexcept = default;
  ArrayRef(ArrayRef const&) noexcept = default;
  ArrayRef(ArrayRef&&) noexcept = default;
  ~ArrayRef() noexcept = default;
  explicit(false) ArrayRef(cds::Array<T>& array) noexcept;
  explicit(false) ArrayRef(std::vector<T>& array) noexcept;
  explicit(false) ArrayRef(T* buffer, cds::Size length) noexcept;
  template <cds::Size size> explicit(false) ArrayRef(cds::StaticArray<T, size>& array) noexcept;
  template <cds::Size size> explicit(false) ArrayRef(std::array<T, size>& array) noexcept;
  template <cds::Size size> explicit(false) ArrayRef(T (&array)[size]) noexcept;
  template <meta::concepts::RandomAccessIterator Iterator> ArrayRef(Iterator&& begin, Iterator&& end) noexcept;
  template <meta::concepts::RandomAccessIterable Iterable> ArrayRef(Iterable&& iterable) noexcept;

  auto operator=(ArrayRef const&) noexcept -> ArrayRef& = default;
  auto operator=(ArrayRef&&) noexcept -> ArrayRef& = default;

  auto operator=(cds::Array<T>& array) noexcept -> ArrayRef&;
  auto operator=(std::vector<T>& array) noexcept -> ArrayRef&;
  template <cds::Size size> auto operator=(T (&array)[size]) noexcept -> ArrayRef&;
  template <cds::Size size> auto operator=(cds::StaticArray<T, size>& array) noexcept -> ArrayRef&;
  template <cds::Size size> auto operator=(std::array<T, size>& array) noexcept -> ArrayRef&;
  template <meta::concepts::RandomAccessIterable Iterable> auto operator=(Iterable&& iterable) noexcept -> ArrayRef&;

  [[nodiscard]] explicit operator bool() const noexcept;

  auto clear() noexcept -> void;
  [[nodiscard]] auto takeFront(cds::Size amount) const noexcept -> ArrayRef;
  [[nodiscard]] auto takeBack(cds::Size amount) const noexcept -> ArrayRef;
  [[nodiscard]] auto dropFront(cds::Size amount) const noexcept -> ArrayRef;
  [[nodiscard]] auto dropBack(cds::Size amount) const noexcept -> ArrayRef;
  auto shrink(cds::Size amount) noexcept -> ArrayRef&;

  [[nodiscard]] constexpr auto operator[](cds::Size index) noexcept -> T& {
    return data()[index];
  }

  [[nodiscard]] constexpr auto operator[](cds::Size index) const noexcept -> T const& {
    return data()[index];
  }

  [[nodiscard]] constexpr auto data() noexcept -> T* { return _buffer; }
  [[nodiscard]] constexpr auto data() const noexcept -> T const* { return _buffer; }
  [[nodiscard]] constexpr auto size() const noexcept { return _size; }
  [[nodiscard]] constexpr auto empty() const noexcept { return data() == nullptr || size() == 0u; }

private:
  T* _buffer {nullptr};
  cds::Size _size {0u};
};

template <typename T> ArrayRef<T>::ArrayRef(cds::Array<T>& array) noexcept :
    ArrayRef(array.data(), array.size()) {}

template <typename T> ArrayRef<T>::ArrayRef(std::vector<T>& array) noexcept :
    ArrayRef(array.data(), array.size()) {}

template <typename T> ArrayRef<T>::ArrayRef(T* buffer, cds::Size length) noexcept :
    _buffer(buffer), _size(length) {}

template <typename T> template <cds::Size size> ArrayRef<T>::ArrayRef(cds::StaticArray<T, size>& array) noexcept :
    ArrayRef(array.data(), array.size()) {}

template <typename T> template <cds::Size size> ArrayRef<T>::ArrayRef(std::array<T, size>& array) noexcept :
    ArrayRef(array.data(), array.size()) {}

template <typename T> template <cds::Size size> ArrayRef<T>::ArrayRef(T (&array)[size]) noexcept :
    ArrayRef(array, size) {}

template <typename T> template <meta::concepts::RandomAccessIterator Iterator> ArrayRef<T>::
    ArrayRef(Iterator&& begin, Iterator&& end) noexcept :
    ArrayRef(&std::forward<Iterator>(begin)[0u], std::forward<Iterator>(end) - std::forward<Iterator>(begin)) {}

template <typename T> template <meta::concepts::RandomAccessIterable Iterable> ArrayRef<T>::
    ArrayRef(Iterable&& iterable) noexcept :
    ArrayRef(std::forward<Iterable>(iterable).begin(), std::forward<Iterable>(iterable).end()) {}

template <typename T> auto ArrayRef<T>::operator=(cds::Array<T>& array) noexcept -> ArrayRef& {
  _buffer = array.data();
  _size = array.size();
  return *this;
}

template <typename T> auto ArrayRef<T>::operator=(std::vector<T>& array) noexcept -> ArrayRef& {
  _buffer = array.data();
  _size = array.size();
  return *this;
}

template <typename T> template <cds::Size size> auto ArrayRef<T>::operator=(T (&array)[size]) noexcept
    -> ArrayRef& {
  _buffer = array;
  _size = size;
  return *this;
}

template <typename T> template <cds::Size size> auto ArrayRef<T>::
    operator=(cds::StaticArray<T, size>& array) noexcept -> ArrayRef& {
  _buffer = array.data();
  _size = array.size();
  return *this;
}

template <typename T> template <cds::Size size> auto ArrayRef<T>::operator=(std::array<T, size>& array) noexcept
    -> ArrayRef& {
  _buffer = array.data();
  _size = array.size();
  return *this;
}

template <typename T> template <meta::concepts::RandomAccessIterable Iterable> auto ArrayRef<T>::
    operator=(Iterable&& iterable) noexcept -> ArrayRef& {
  _buffer = &std::forward<Iterable>(iterable.begin())[0u];
  _size = std::forward<Iterable>(iterable).end() - std::forward<Iterable>(iterable).begin();
  return *this;
}

template <typename T> ArrayRef<T>::operator bool() const noexcept {
  return !empty();
}

template <typename T> auto ArrayRef<T>::clear() noexcept -> void {
  _buffer = nullptr;
  _size = 0u;
}

template <typename T> auto ArrayRef<T>::takeFront(cds::Size amount) const noexcept -> ArrayRef {
  if (amount >= size()) {
    return *this;
  }
  return {data(), amount};
}

template <typename T> auto ArrayRef<T>::takeBack(cds::Size amount) const noexcept -> ArrayRef {
  if (amount >= size()) {
    return *this;
  }
  return {data() + size() - amount, amount};
}

template <typename T> auto ArrayRef<T>::dropFront(cds::Size amount) const noexcept -> ArrayRef {
  if (amount >= size()) {
    return {};
  }
  return {data() + amount, size() - amount};
}

template <typename T> auto ArrayRef<T>::dropBack(cds::Size amount) const noexcept -> ArrayRef {
  if (amount >= size()) {
    return {};
  }
  return {data(), size() - amount};
}
} // namespace age
