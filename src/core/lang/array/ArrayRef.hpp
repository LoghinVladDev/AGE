//
// Created by Vlad-Andrei Loghin on 12.07.23.
//

#pragma once
#include <array>
#include <vector>

#include <CDS/Array>
#include <CDS/StaticArray>
#include <lang/generic/Concepts.hpp>
#include <lang/generic/OperatorsWithImplicitConstruction.hpp>

namespace age {
template <typename T> class ArrayRef {
public:
  ArrayRef() noexcept = default;
  ArrayRef(ArrayRef const&) noexcept = default;
  ArrayRef(ArrayRef&&) noexcept = default;
  ~ArrayRef() noexcept = default;
  explicit(false) ArrayRef(cds::Array<T>& array) noexcept;
  explicit(false) ArrayRef(std::vector<T>& array) noexcept;
  explicit(false) ArrayRef(T* buffer, cds::Size length) noexcept;
  template <cds::Size size> explicit(false) ArrayRef(cds::StaticArray<T, size>& array) noexcept;
  template <std::size_t size> explicit(false) ArrayRef(std::array<T, size>& array) noexcept;
  template <cds::Size size> explicit(false) ArrayRef(T (&array)[size]) noexcept;
  ArrayRef(std::initializer_list<T> const& list) noexcept
    requires meta::concepts::ConstQualified<T>
      : ArrayRef(list.begin(), list.end()) {}

  template <meta::concepts::RandomAccessIterator Iterator> ArrayRef(Iterator&& begin, Iterator&& end) noexcept :
      ArrayRef(&std::forward<Iterator>(begin)[0u], std::forward<Iterator>(end) - std::forward<Iterator>(begin)) {}

  template <meta::concepts::RandomAccessIterable Iterable>
    requires(!cds::meta::IsSame<Iterable, ArrayRef<T>>::value)
  ArrayRef(Iterable&& iterable) noexcept :
      ArrayRef(std::forward<Iterable>(iterable).begin(), std::forward<Iterable>(iterable).end()) {}

  auto operator=(ArrayRef const&) noexcept -> ArrayRef& = default;
  auto operator=(ArrayRef&&) noexcept -> ArrayRef& = default;

  auto operator=(cds::Array<T>& array) noexcept -> ArrayRef&;
  auto operator=(std::vector<T>& array) noexcept -> ArrayRef&;
  template <cds::Size size> auto operator=(T (&array)[size]) noexcept -> ArrayRef&;
  template <cds::Size size> auto operator=(cds::StaticArray<T, size>& array) noexcept -> ArrayRef&;
  template <std::size_t size> auto operator=(std::array<T, size>& array) noexcept -> ArrayRef&;

  template <meta::concepts::RandomAccessIterable Iterable>
    requires(!cds::meta::IsSame<Iterable, ArrayRef<T>>::value)
  auto operator=(Iterable&& iterable) noexcept -> ArrayRef& {
    _buffer = &std::forward<Iterable>(iterable.begin())[0u];
    _size = std::forward<Iterable>(iterable).end() - std::forward<Iterable>(iterable).begin();
    return *this;
  }

  [[nodiscard]] explicit operator bool() const noexcept;

  auto clear() noexcept -> void;
  [[nodiscard]] auto takeFront(cds::Size amount) const noexcept -> ArrayRef;
  [[nodiscard]] auto takeBack(cds::Size amount) const noexcept -> ArrayRef;
  [[nodiscard]] auto dropFront(cds::Size amount) const noexcept -> ArrayRef;
  [[nodiscard]] auto dropBack(cds::Size amount) const noexcept -> ArrayRef;
  auto shrink(cds::Size amount) noexcept -> ArrayRef&;

  [[nodiscard]] constexpr auto operator[](cds::Size index) noexcept -> T& { return data()[index]; }
  [[nodiscard]] constexpr auto operator[](cds::Size index) const noexcept -> T const& { return data()[index]; }

  [[nodiscard]] constexpr auto data() noexcept -> T* { return _buffer; }
  [[nodiscard]] constexpr auto data() const noexcept -> T const* { return _buffer; }
  [[nodiscard]] constexpr auto size() const noexcept { return _size; }
  [[nodiscard]] constexpr auto empty() const noexcept { return data() == nullptr || size() == 0u; }

  [[nodiscard]] constexpr auto begin() noexcept -> T* { return _buffer; }
  [[nodiscard]] constexpr auto end() noexcept -> T* { return _buffer + _size; }
  [[nodiscard]] constexpr auto cbegin() const noexcept -> T const* { return _buffer; }
  [[nodiscard]] constexpr auto cend() const noexcept -> T const* { return _buffer + _size; }

  [[nodiscard]] constexpr auto operator==(std::initializer_list<T> const& list) const noexcept {
    return equal(list.begin(), list.end());
  }

  template <meta::concepts::ForwardIterator Iterator, meta::concepts::SentinelFor<Iterator> Sentinel>
  [[nodiscard]] constexpr auto equal(Iterator iterator, Sentinel const& sentinel) const noexcept {
    auto lIt = _buffer;
    auto lEnd = _buffer + _size;
    for (; lIt != lEnd && iterator != sentinel; ++lIt, ++iterator) {
      if (!cds::meta::equals(*lIt, *iterator)) {
        return false;
      }
    }
    return lIt == lEnd && iterator == sentinel;
  }

private:
  T* _buffer {nullptr};
  cds::Size _size {0u};
};

template <typename T> auto ref(cds::Array<T>& array) noexcept { return ArrayRef(array); }
template <typename T> auto ref(std::vector<T>& array) noexcept { return ArrayRef(array); }
template <typename T, cds::Size size> auto ref(cds::StaticArray<T, size>& array) noexcept { return ArrayRef(array); }
template <typename T, std::size_t size> auto ref(std::array<T, size>& array) noexcept { return ArrayRef(array); }

template <typename T> ArrayRef<T>::ArrayRef(cds::Array<T>& array) noexcept : ArrayRef(array.data(), array.size()) {}
template <typename T> ArrayRef<T>::ArrayRef(std::vector<T>& array) noexcept : ArrayRef(array.data(), array.size()) {}
template <typename T> ArrayRef<T>::ArrayRef(T* buffer, cds::Size length) noexcept : _buffer(buffer), _size(length) {}

template <typename T> template <cds::Size size> ArrayRef<T>::ArrayRef(cds::StaticArray<T, size>& array) noexcept :
    ArrayRef(array.data(), array.size()) {}

template <typename T> template <std::size_t size> ArrayRef<T>::ArrayRef(std::array<T, size>& array) noexcept :
    ArrayRef(array.data(), array.size()) {}

template <typename T> template <cds::Size size> ArrayRef<T>::ArrayRef(T (&array)[size]) noexcept :
    ArrayRef(array, size) {}

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

template <typename T> template <cds::Size size> auto ArrayRef<T>::operator=(T (&array)[size]) noexcept -> ArrayRef& {
  _buffer = array;
  _size = size;
  return *this;
}

template <typename T> template <cds::Size size> auto ArrayRef<T>::operator=(cds::StaticArray<T, size>& array) noexcept
    -> ArrayRef& {
  _buffer = array.data();
  _size = array.size();
  return *this;
}

template <typename T> template <std::size_t size> auto ArrayRef<T>::operator=(std::array<T, size>& array) noexcept
    -> ArrayRef& {
  _buffer = array.data();
  _size = array.size();
  return *this;
}

template <typename T> ArrayRef<T>::operator bool() const noexcept { return !empty(); }

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

template <typename T> ArrayRef(cds::Array<T>&) -> ArrayRef<T>;
template <typename T> ArrayRef(std::vector<T>&) -> ArrayRef<T>;
template <typename T, cds::Size size> ArrayRef(cds::StaticArray<T, size>&) -> ArrayRef<T>;
template <typename T, cds::Size size> ArrayRef(std::array<T, size>&) -> ArrayRef<T>;

template <typename T> ArrayRef(cds::Array<T> const&) -> ArrayRef<T const>;
template <typename T> ArrayRef(std::vector<T> const&) -> ArrayRef<T const>;
template <typename T, cds::Size size> ArrayRef(cds::StaticArray<T, size> const&) -> ArrayRef<T const>;
template <typename T, cds::Size size> ArrayRef(std::array<T, size> const&) -> ArrayRef<T const>;
template <typename T> ArrayRef(std::initializer_list<T>) -> ArrayRef<T const>;
} // namespace age