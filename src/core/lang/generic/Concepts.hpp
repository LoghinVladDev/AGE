//
// Created by Vlad-Andrei Loghin on 17.07.23.
//

#pragma once

#include <CDS/meta/TypeTraits>

namespace age::meta::concepts {
namespace detail {
template <typename L, typename R>
concept SameAs = cds::meta::IsSame<L, R>::value;

template <typename T> using Dereferenceable = decltype(*cds::meta::referenceOf<T>());
} // namespace detail

template <typename T>
concept Dereferenceable = requires(T i) { typename detail::Dereferenceable<T>; };

template <typename L, typename R>
concept SameAs = detail::SameAs<L, R> && detail::SameAs<R, L>;

template <typename L, typename R>
concept ConvertibleTo = cds::meta::IsConvertible<L, R>::value;

template <typename T>
concept WeaklyIncrementable = requires(T i) {
  { ++i } -> SameAs<T&>;
};

template <typename T>
concept WeaklyDecrementable = requires(T i) {
  { --i } -> SameAs<T&>;
};

template <typename S, typename I>
concept SentinelFor = cds::meta::EqualToPossible<S, I>::value;

template <typename T>
concept ForwardIterator = WeaklyIncrementable<T> && Dereferenceable<T> && SentinelFor<T, T>;

template <typename T>
concept BidirectionalIterator = ForwardIterator<T> && WeaklyDecrementable<T>;

template <typename T, typename U>
concept PartiallyOrderedWith = requires(T l, U r) {
  { l < r } -> ConvertibleTo<bool>;
  { l > r } -> ConvertibleTo<bool>;
  { l <= r } -> ConvertibleTo<bool>;
  { l >= r } -> ConvertibleTo<bool>;
  { r < l } -> ConvertibleTo<bool>;
  { r > l } -> ConvertibleTo<bool>;
  { r <= l } -> ConvertibleTo<bool>;
  { r >= l } -> ConvertibleTo<bool>;
};

template <typename T>
concept TotallyOrdered = cds::meta::EqualToPossible<T, T>::value && PartiallyOrderedWith<T, T>;

template <typename T>
concept RandomAccessIterator =
    BidirectionalIterator<T> && TotallyOrdered<T> && requires(T i, T const j, decltype(i - j) const n) {
      { i += n } -> SameAs<T&>;
      { j + n } -> SameAs<T>;
      { n + j } -> SameAs<T>;
      { i -= n } -> SameAs<T&>;
      { j - n } -> SameAs<T>;
      { j[n] } -> SameAs<decltype(*i)>;
    };

template <typename T>
concept ForwardIterable = requires(T const& obj) {
  { obj.begin() } -> ForwardIterator;
  { obj.end() } -> ForwardIterator;
};

template <typename T>
concept BidirectionalIterable = requires(T const& obj) {
  { obj.begin() } -> BidirectionalIterator;
  { obj.end() } -> BidirectionalIterator;
};

template <typename T>
concept RandomAccessIterable = requires(T const& obj) {
  { obj.begin() } -> RandomAccessIterator;
  { obj.end() } -> RandomAccessIterator;
};
} // namespace age::meta::concepts