//
// Created by Vlad-Andrei Loghin on 27.07.23.
//

#include <core/lang/generic/Concepts.hpp>
#include <CDS/meta/TypeTraits>
#include <CDS/LinkedList>
#include <CDS/Array>
#include <CDS/StaticArray>
#include <CDS/HashSet>
#include <CDS/TreeSet>
#include <CDS/LinkedHashSet>
#include <CDS/HashMap>
#include <CDS/TreeMap>
#include <CDS/LinkedHashMap>
#include <vector>
#include <list>
#include <array>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace {
using namespace age::meta::concepts;

namespace iteratorModels {

template <typename = void>
struct ForwardIterator_E {};

template <typename = void>
struct ForwardIterator_Drf {
  int operator*() {}
};

template <typename R = void>
struct ForwardIterator_Inc {
  cds::meta::Conditional<cds::meta::IsSame<R, void>::value, ForwardIterator_Inc&, R> operator++();
};

template <typename = void>
struct ForwardIterator_Eq {
  bool operator==(ForwardIterator_Eq const&) const;
};

template <typename R = void>
struct ForwardIterator_Drf_Inc {
  int operator*() {}
  cds::meta::Conditional<cds::meta::IsSame<R, void>::value, ForwardIterator_Drf_Inc&, R> operator++();
};

template <typename = void>
struct ForwardIterator_Drf_Eq {
  int operator*() {}
  bool operator==(ForwardIterator_Drf_Eq const&) const;
};

template <typename = void>
struct ForwardIterator_Inc_Eq {
  ForwardIterator_Inc_Eq& operator++();
  bool operator==(ForwardIterator_Inc_Eq const&) const;
};

template <typename R = void>
struct ForwardIterator {
  int operator*() {}
  cds::meta::Conditional<cds::meta::IsSame<R, void>::value, ForwardIterator&, R> operator++();
  bool operator==(ForwardIterator const&) const;
};

template <template <typename...> class Base>
struct BidirectionalIterator_E : Base<BidirectionalIterator_E<Base>&> {};

template <template <typename...> class Base, typename R = void>
struct BidirectionalIterator : Base<BidirectionalIterator<Base>&> {
  cds::meta::Conditional<cds::meta::IsSame<void, R>::value, BidirectionalIterator&, R> operator--();
};

struct RandomAccessIterator_E : BidirectionalIterator_E<ForwardIterator_E> {};

struct RandomAccessIterator : BidirectionalIterator<ForwardIterator> {
  RandomAccessIterator& operator++();
  RandomAccessIterator& operator--();
  bool operator==(RandomAccessIterator const&) const;

  std::strong_ordering operator<=>(RandomAccessIterator const&) const;
  RandomAccessIterator& operator+=(int);
  RandomAccessIterator operator+(int) const;
  friend RandomAccessIterator operator+(int, RandomAccessIterator const&);
  RandomAccessIterator& operator-=(int);
  RandomAccessIterator operator-(int) const;
  int operator-(RandomAccessIterator const&) const;
  int operator[](int) const;
};
} // namespace iteratorModels
} // namespace

consteval auto isSameTest() {
  static_assert(SameAs<int, int>);
  static_assert(!SameAs<int, float>);
}

consteval auto dereferenceableTest() {
  struct ND{};
  struct D{public: auto operator*() {}};
  static_assert(!Dereferenceable<int>);
  static_assert(Dereferenceable<int*>);
  static_assert(!Dereferenceable<ND>);
  static_assert(Dereferenceable<D>);
}

consteval auto convertibleToTest() {
  static_assert(ConvertibleTo<int, float>);
  static_assert(ConvertibleTo<float, int>);
  static_assert(ConvertibleTo<int, int>);
  static_assert(!ConvertibleTo<int, int*>);
  static_assert(!ConvertibleTo<int*, int>);
}

consteval auto weaklyIncrementableTest() {
  struct NInc {};
  struct IncNWeakly {auto operator++();};
  struct IncWeakly {auto& operator++(){return *this;}};
  static_assert(WeaklyIncrementable<int>);
  static_assert(!WeaklyIncrementable<void>);
  static_assert(!WeaklyIncrementable<NInc>);
  static_assert(!WeaklyIncrementable<IncNWeakly>);
  static_assert(WeaklyIncrementable<IncWeakly>);
}

consteval auto weaklyDecrementableTest() {
  struct NDec {};
  struct DecNWeakly {auto operator--();};
  struct DecWeakly {auto& operator--(){return *this;}};
  static_assert(WeaklyDecrementable<int>);
  static_assert(!WeaklyDecrementable<void>);
  static_assert(!WeaklyDecrementable<NDec>);
  static_assert(!WeaklyDecrementable<DecNWeakly>);
  static_assert(WeaklyDecrementable<DecWeakly>);
}

consteval auto sentinelForTest() {
  struct RelatedSentinel {};
  struct UnrelatedSentinel {};
  struct Iterator {bool operator==(RelatedSentinel const&) const;};
  static_assert(SentinelFor<Iterator, RelatedSentinel>);
  static_assert(!SentinelFor<Iterator, UnrelatedSentinel>);
}

consteval auto forwardIteratorTest() {
  static_assert(!ForwardIterator<int>);
  static_assert(!ForwardIterator<void>);
  static_assert(!ForwardIterator<void*>);
  static_assert(ForwardIterator<int*>);

  static_assert(!ForwardIterator<iteratorModels::ForwardIterator_E<>>);
  static_assert(!ForwardIterator<iteratorModels::ForwardIterator_Drf<>>);
  static_assert(!ForwardIterator<iteratorModels::ForwardIterator_Inc<>>);
  static_assert(!ForwardIterator<iteratorModels::ForwardIterator_Eq<>>);
  static_assert(!ForwardIterator<iteratorModels::ForwardIterator_Drf_Inc<>>);
  static_assert(!ForwardIterator<iteratorModels::ForwardIterator_Drf_Eq<>>);
  static_assert(!ForwardIterator<iteratorModels::ForwardIterator_Inc_Eq<>>);
  static_assert(ForwardIterator<iteratorModels::ForwardIterator<>>);

  static_assert(ForwardIterator<cds::Iterable<int>::ConstIterator>);
  static_assert(ForwardIterator<cds::Collection<int>::ConstIterator>);
  static_assert(ForwardIterator<cds::MutableCollection<int>::Iterator>);
  static_assert(ForwardIterator<cds::MutableCollection<int>::ConstIterator>);
  static_assert(ForwardIterator<cds::List<int>::Iterator>);
  static_assert(ForwardIterator<cds::List<int>::ReverseIterator>);
  static_assert(ForwardIterator<cds::List<int>::ConstReverseIterator>);
  static_assert(ForwardIterator<cds::List<int>::ConstIterator>);
  static_assert(ForwardIterator<cds::Set<int>::ConstIterator>);
  static_assert(ForwardIterator<cds::Map<int, int>::Iterator>);
  static_assert(ForwardIterator<cds::Map<int, int>::ConstIterator>);

  static_assert(ForwardIterator<cds::Array<int>::Iterator>);
  static_assert(ForwardIterator<cds::Array<int>::ConstIterator>);
  static_assert(ForwardIterator<cds::Array<int>::ReverseIterator>);
  static_assert(ForwardIterator<cds::Array<int>::ConstReverseIterator>);

  static_assert(ForwardIterator<cds::LinkedList<int>::Iterator>);
  static_assert(ForwardIterator<cds::LinkedList<int>::ConstIterator>);
  static_assert(ForwardIterator<cds::LinkedList<int>::ReverseIterator>);
  static_assert(ForwardIterator<cds::LinkedList<int>::ConstReverseIterator>);

  static_assert(ForwardIterator<cds::StaticArray<int, 5>::Iterator>);
  static_assert(ForwardIterator<cds::StaticArray<int, 5>::ConstIterator>);
  static_assert(ForwardIterator<cds::StaticArray<int, 5>::ReverseIterator>);
  static_assert(ForwardIterator<cds::StaticArray<int, 5>::ConstReverseIterator>);

  static_assert(ForwardIterator<cds::HashSet<int>::ConstIterator>);

  static_assert(!ForwardIterator<cds::TreeSet<int>::ConstIterator>); // FIXME SP/LV investigate lack of weakly inc. LV ConstIterator = Iterator;
  static_assert(ForwardIterator<cds::TreeSet<int>::ConstReverseIterator>);

  static_assert(ForwardIterator<cds::LinkedHashSet<int>::ConstIterator>);

  static_assert(ForwardIterator<cds::HashMap<int, int>::Iterator>);
  static_assert(ForwardIterator<cds::HashMap<int, int>::ConstIterator>);

  static_assert(ForwardIterator<cds::TreeMap<int, int>::Iterator>);
  static_assert(ForwardIterator<cds::TreeMap<int, int>::ConstIterator>);
  static_assert(ForwardIterator<cds::TreeMap<int, int>::ReverseIterator>);
  static_assert(ForwardIterator<cds::TreeMap<int, int>::ConstReverseIterator>);

  static_assert(ForwardIterator<cds::LinkedHashMap<int, int>::Iterator>);
  static_assert(ForwardIterator<cds::LinkedHashMap<int, int>::ConstIterator>);


  static_assert(ForwardIterator<std::vector<int>::iterator>);
  static_assert(ForwardIterator<std::vector<int>::const_iterator>);
  static_assert(ForwardIterator<std::vector<int>::reverse_iterator>);
  static_assert(ForwardIterator<std::vector<int>::const_reverse_iterator>);

  static_assert(ForwardIterator<std::array<int, 5>::iterator>);
  static_assert(ForwardIterator<std::array<int, 5>::const_iterator>);
  static_assert(ForwardIterator<std::array<int, 5>::reverse_iterator>);
  static_assert(ForwardIterator<std::array<int, 5>::const_reverse_iterator>);

  static_assert(ForwardIterator<std::list<int>::iterator>);
  static_assert(ForwardIterator<std::list<int>::const_iterator>);
  static_assert(ForwardIterator<std::list<int>::reverse_iterator>);
  static_assert(ForwardIterator<std::list<int>::const_reverse_iterator>);

  static_assert(ForwardIterator<std::set<int>::iterator>);
  static_assert(ForwardIterator<std::set<int>::const_iterator>);
  static_assert(ForwardIterator<std::set<int>::reverse_iterator>);
  static_assert(ForwardIterator<std::set<int>::const_reverse_iterator>);

  static_assert(ForwardIterator<std::unordered_set<int>::iterator>);
  static_assert(ForwardIterator<std::unordered_set<int>::const_iterator>);

  static_assert(ForwardIterator<std::map<int, int>::iterator>);
  static_assert(ForwardIterator<std::map<int, int>::const_iterator>);
  static_assert(ForwardIterator<std::map<int, int>::reverse_iterator>);
  static_assert(ForwardIterator<std::map<int, int>::const_reverse_iterator>);

  static_assert(ForwardIterator<std::unordered_map<int, int>::iterator>);
  static_assert(ForwardIterator<std::unordered_map<int, int>::const_iterator>);
}

consteval auto bidirectionalIteratorTest() {
  static_assert(!BidirectionalIterator<int>);
  static_assert(!BidirectionalIterator<void>);
  static_assert(!BidirectionalIterator<void*>);
  static_assert(BidirectionalIterator<int*>);


  static_assert(!BidirectionalIterator<iteratorModels::BidirectionalIterator_E<iteratorModels::ForwardIterator_E>>);
  static_assert(!BidirectionalIterator<iteratorModels::BidirectionalIterator_E<iteratorModels::ForwardIterator_Drf>>);
  static_assert(!BidirectionalIterator<iteratorModels::BidirectionalIterator_E<iteratorModels::ForwardIterator_Inc>>);
  static_assert(!BidirectionalIterator<iteratorModels::BidirectionalIterator_E<iteratorModels::ForwardIterator_Eq>>);
  static_assert(!BidirectionalIterator<iteratorModels::BidirectionalIterator_E<iteratorModels::ForwardIterator_Drf_Inc>>);
  static_assert(!BidirectionalIterator<iteratorModels::BidirectionalIterator_E<iteratorModels::ForwardIterator_Drf_Eq>>);
  static_assert(!BidirectionalIterator<iteratorModels::BidirectionalIterator_E<iteratorModels::ForwardIterator_Inc_Eq>>);
  static_assert(!BidirectionalIterator<iteratorModels::BidirectionalIterator_E<iteratorModels::ForwardIterator>>);

  static_assert(!BidirectionalIterator<iteratorModels::BidirectionalIterator<iteratorModels::ForwardIterator_E>>);
  static_assert(!BidirectionalIterator<iteratorModels::BidirectionalIterator<iteratorModels::ForwardIterator_Drf>>);
  static_assert(!BidirectionalIterator<iteratorModels::BidirectionalIterator<iteratorModels::ForwardIterator_Inc>>);
  static_assert(!BidirectionalIterator<iteratorModels::BidirectionalIterator<iteratorModels::ForwardIterator_Eq>>);
  static_assert(!BidirectionalIterator<iteratorModels::BidirectionalIterator<iteratorModels::ForwardIterator_Drf_Inc>>);
  static_assert(!BidirectionalIterator<iteratorModels::BidirectionalIterator<iteratorModels::ForwardIterator_Drf_Eq>>);
  static_assert(!BidirectionalIterator<iteratorModels::BidirectionalIterator<iteratorModels::ForwardIterator_Inc_Eq>>);
  static_assert(BidirectionalIterator<iteratorModels::BidirectionalIterator<iteratorModels::ForwardIterator>>);

  static_assert(!BidirectionalIterator<cds::Iterable<int>::ConstIterator>);
  static_assert(!BidirectionalIterator<cds::Collection<int>::ConstIterator>);
  static_assert(!BidirectionalIterator<cds::MutableCollection<int>::Iterator>);
  static_assert(!BidirectionalIterator<cds::MutableCollection<int>::ConstIterator>);
  static_assert(BidirectionalIterator<cds::List<int>::Iterator>);
  static_assert(BidirectionalIterator<cds::List<int>::ReverseIterator>);
  static_assert(BidirectionalIterator<cds::List<int>::ConstReverseIterator>);
  static_assert(BidirectionalIterator<cds::List<int>::ConstIterator>);
  static_assert(!BidirectionalIterator<cds::Set<int>::ConstIterator>);
  static_assert(!BidirectionalIterator<cds::Map<int, int>::Iterator>);
  static_assert(!BidirectionalIterator<cds::Map<int, int>::ConstIterator>);

  static_assert(BidirectionalIterator<cds::Array<int>::Iterator>);
  static_assert(BidirectionalIterator<cds::Array<int>::ConstIterator>);
  static_assert(BidirectionalIterator<cds::Array<int>::ReverseIterator>);
  static_assert(BidirectionalIterator<cds::Array<int>::ConstReverseIterator>);

  static_assert(BidirectionalIterator<cds::LinkedList<int>::Iterator>);
  static_assert(BidirectionalIterator<cds::LinkedList<int>::ConstIterator>);
  static_assert(BidirectionalIterator<cds::LinkedList<int>::ReverseIterator>);
  static_assert(BidirectionalIterator<cds::LinkedList<int>::ConstReverseIterator>);

  static_assert(BidirectionalIterator<cds::StaticArray<int, 5>::Iterator>);
  static_assert(BidirectionalIterator<cds::StaticArray<int, 5>::ConstIterator>);
  static_assert(BidirectionalIterator<cds::StaticArray<int, 5>::ReverseIterator>);
  static_assert(BidirectionalIterator<cds::StaticArray<int, 5>::ConstReverseIterator>);

  static_assert(!BidirectionalIterator<cds::HashSet<int>::ConstIterator>);

  static_assert(!BidirectionalIterator<cds::TreeSet<int>::ConstIterator>); // FIXME SP/LV investigate lack of weakly inc. LV ConstIterator = Iterator;
  static_assert(BidirectionalIterator<cds::TreeSet<int>::ConstReverseIterator>);

  static_assert(!BidirectionalIterator<cds::LinkedHashSet<int>::ConstIterator>);

  static_assert(!BidirectionalIterator<cds::HashMap<int, int>::Iterator>);
  static_assert(!BidirectionalIterator<cds::HashMap<int, int>::ConstIterator>);

  static_assert(BidirectionalIterator<cds::TreeMap<int, int>::Iterator>);
  static_assert(BidirectionalIterator<cds::TreeMap<int, int>::ConstIterator>);
  static_assert(BidirectionalIterator<cds::TreeMap<int, int>::ReverseIterator>);
  static_assert(BidirectionalIterator<cds::TreeMap<int, int>::ConstReverseIterator>);

  static_assert(!BidirectionalIterator<cds::LinkedHashMap<int, int>::Iterator>);
  static_assert(!BidirectionalIterator<cds::LinkedHashMap<int, int>::ConstIterator>);


  static_assert(BidirectionalIterator<std::vector<int>::iterator>);
  static_assert(BidirectionalIterator<std::vector<int>::const_iterator>);
  static_assert(BidirectionalIterator<std::vector<int>::reverse_iterator>);
  static_assert(BidirectionalIterator<std::vector<int>::const_reverse_iterator>);

  static_assert(BidirectionalIterator<std::array<int, 5>::iterator>);
  static_assert(BidirectionalIterator<std::array<int, 5>::const_iterator>);
  static_assert(BidirectionalIterator<std::array<int, 5>::reverse_iterator>);
  static_assert(BidirectionalIterator<std::array<int, 5>::const_reverse_iterator>);

  static_assert(BidirectionalIterator<std::list<int>::iterator>);
  static_assert(BidirectionalIterator<std::list<int>::const_iterator>);
  static_assert(BidirectionalIterator<std::list<int>::reverse_iterator>);
  static_assert(BidirectionalIterator<std::list<int>::const_reverse_iterator>);

  static_assert(BidirectionalIterator<std::set<int>::iterator>);
  static_assert(BidirectionalIterator<std::set<int>::const_iterator>);
  static_assert(BidirectionalIterator<std::set<int>::reverse_iterator>);
  static_assert(BidirectionalIterator<std::set<int>::const_reverse_iterator>);

  static_assert(!BidirectionalIterator<std::unordered_set<int>::iterator>);
  static_assert(!BidirectionalIterator<std::unordered_set<int>::const_iterator>);

  static_assert(BidirectionalIterator<std::map<int, int>::iterator>);
  static_assert(BidirectionalIterator<std::map<int, int>::const_iterator>);
  static_assert(BidirectionalIterator<std::map<int, int>::reverse_iterator>);
  static_assert(BidirectionalIterator<std::map<int, int>::const_reverse_iterator>);

  static_assert(!BidirectionalIterator<std::unordered_map<int, int>::iterator>);
  static_assert(!BidirectionalIterator<std::unordered_map<int, int>::const_iterator>);
}

consteval auto partiallyOrderedWithTest() {
  struct OthOrd {};
  struct SelfOrd {
    std::strong_ordering operator<=>(SelfOrd const&) const;
    std::strong_ordering operator<=>(OthOrd) const;
  };

  struct NoCmp {};
  struct OnlyGr {
    bool operator>(OnlyGr const&) const;
  };

  struct OnlyGrLe {
    bool operator>(OnlyGrLe const&) const;
    bool operator<(OnlyGrLe const&) const;
  };

  struct OnlyGreLee {
    bool operator>=(OnlyGreLee const&) const;
    bool operator<=(OnlyGreLee const&) const;
  };

  struct All {
    bool operator>(All const&) const;
    bool operator<(All const&) const;
    bool operator>=(All const&) const;
    bool operator<=(All const&) const;
  };

  static_assert(PartiallyOrderedWith<int, int>);
  static_assert(PartiallyOrderedWith<int, float>);
  static_assert(PartiallyOrderedWith<float, int>);
  static_assert(!PartiallyOrderedWith<int, void>);
  static_assert(PartiallyOrderedWith<SelfOrd, SelfOrd>);
  static_assert(!PartiallyOrderedWith<SelfOrd, int>);
  static_assert(!PartiallyOrderedWith<int, SelfOrd>);
  static_assert(PartiallyOrderedWith<OthOrd, SelfOrd>);
  static_assert(PartiallyOrderedWith<SelfOrd, OthOrd>);
  static_assert(!PartiallyOrderedWith<NoCmp, NoCmp>);
  static_assert(!PartiallyOrderedWith<OnlyGr, OnlyGr>);
  static_assert(!PartiallyOrderedWith<OnlyGrLe, OnlyGrLe>);
  static_assert(!PartiallyOrderedWith<OnlyGreLee, OnlyGreLee>);
  static_assert(PartiallyOrderedWith<All, All>);
}

consteval auto totallyOrderedTest() {
  struct TO1 {
    bool operator==(TO1 const&) const;
    bool operator!=(TO1 const&) const;
    bool operator<(TO1 const&) const;
    bool operator>(TO1 const&) const;
    bool operator<=(TO1 const&) const;
    bool operator>=(TO1 const&) const;
  };

  struct TO2 {
    bool operator==(TO2 const&) const;
    std::strong_ordering operator<=>(TO2 const&) const;
  };

  struct TO_Inc1 {};
  struct TO_Inc2 {
    bool operator !=(TO_Inc2 const&) const;
  };

  struct TO_Inc3 {
    bool operator ==(TO_Inc3 const&) const;
    bool operator <(TO_Inc3 const&) const;
    bool operator <=(TO_Inc3 const&) const;
  };

  static_assert(TotallyOrdered<int>);
  static_assert(TotallyOrdered<float>);
  static_assert(!TotallyOrdered<void>);
  static_assert(TotallyOrdered<void*>);
  static_assert(TotallyOrdered<TO1>);
  static_assert(TotallyOrdered<TO2>);
  static_assert(!TotallyOrdered<TO_Inc1>);
  static_assert(!TotallyOrdered<TO_Inc2>);
  static_assert(!TotallyOrdered<TO_Inc3>);
}

consteval auto randomAccessIteratorTest() {
  static_assert(RandomAccessIterator<int*>);
  static_assert(!RandomAccessIterator<void*>);
  static_assert(!RandomAccessIterator<int>);
  static_assert(!RandomAccessIterator<void>);
  static_assert(!RandomAccessIterator<iteratorModels::RandomAccessIterator_E>);
  static_assert(RandomAccessIterator<iteratorModels::RandomAccessIterator>);

  static_assert(!RandomAccessIterator<cds::Iterable<int>::ConstIterator>);
  static_assert(!RandomAccessIterator<cds::Collection<int>::ConstIterator>);
  static_assert(!RandomAccessIterator<cds::MutableCollection<int>::Iterator>);
  static_assert(!RandomAccessIterator<cds::MutableCollection<int>::ConstIterator>);
  static_assert(!RandomAccessIterator<cds::List<int>::Iterator>);
  static_assert(!RandomAccessIterator<cds::List<int>::ReverseIterator>);
  static_assert(!RandomAccessIterator<cds::List<int>::ConstReverseIterator>);
  static_assert(!RandomAccessIterator<cds::List<int>::ConstIterator>);
  static_assert(!RandomAccessIterator<cds::Set<int>::ConstIterator>);
  static_assert(!RandomAccessIterator<cds::Map<int, int>::Iterator>);
  static_assert(!RandomAccessIterator<cds::Map<int, int>::ConstIterator>);

  static_assert(!RandomAccessIterator<cds::Array<int>::Iterator>); // FIXME LV : add op +=
  static_assert(!RandomAccessIterator<cds::Array<int>::ConstIterator>); // FIXME LV : add op +=
  static_assert(!RandomAccessIterator<cds::Array<int>::ReverseIterator>); // FIXME LV : add op +=
  static_assert(!RandomAccessIterator<cds::Array<int>::ConstReverseIterator>); // FIXME LV : add op +=

  static_assert(!RandomAccessIterator<cds::LinkedList<int>::Iterator>);
  static_assert(!RandomAccessIterator<cds::LinkedList<int>::ConstIterator>);
  static_assert(!RandomAccessIterator<cds::LinkedList<int>::ReverseIterator>);
  static_assert(!RandomAccessIterator<cds::LinkedList<int>::ConstReverseIterator>);

  static_assert(!RandomAccessIterator<cds::StaticArray<int, 5>::Iterator>); // FIXME LV : add op +=
  static_assert(!RandomAccessIterator<cds::StaticArray<int, 5>::ConstIterator>); // FIXME LV : add op +=
  static_assert(!RandomAccessIterator<cds::StaticArray<int, 5>::ReverseIterator>); // FIXME LV : add op +=
  static_assert(!RandomAccessIterator<cds::StaticArray<int, 5>::ConstReverseIterator>); // FIXME LV : add op +=

  static_assert(!RandomAccessIterator<cds::HashSet<int>::ConstIterator>);

  static_assert(!RandomAccessIterator<cds::TreeSet<int>::ConstIterator>);
  static_assert(!RandomAccessIterator<cds::TreeSet<int>::ConstReverseIterator>);

  static_assert(!RandomAccessIterator<cds::LinkedHashSet<int>::ConstIterator>);

  static_assert(!RandomAccessIterator<cds::HashMap<int, int>::Iterator>);
  static_assert(!RandomAccessIterator<cds::HashMap<int, int>::ConstIterator>);

  static_assert(!RandomAccessIterator<cds::TreeMap<int, int>::Iterator>);
  static_assert(!RandomAccessIterator<cds::TreeMap<int, int>::ConstIterator>);
  static_assert(!RandomAccessIterator<cds::TreeMap<int, int>::ReverseIterator>);
  static_assert(!RandomAccessIterator<cds::TreeMap<int, int>::ConstReverseIterator>);

  static_assert(!RandomAccessIterator<cds::LinkedHashMap<int, int>::Iterator>);
  static_assert(!RandomAccessIterator<cds::LinkedHashMap<int, int>::ConstIterator>);


  static_assert(RandomAccessIterator<std::vector<int>::iterator>);
  static_assert(RandomAccessIterator<std::vector<int>::const_iterator>);
  static_assert(RandomAccessIterator<std::vector<int>::reverse_iterator>);
  static_assert(RandomAccessIterator<std::vector<int>::const_reverse_iterator>);

  static_assert(RandomAccessIterator<std::array<int, 5>::iterator>);
  static_assert(RandomAccessIterator<std::array<int, 5>::const_iterator>);
  static_assert(RandomAccessIterator<std::array<int, 5>::reverse_iterator>);
  static_assert(RandomAccessIterator<std::array<int, 5>::const_reverse_iterator>);

  static_assert(!RandomAccessIterator<std::list<int>::iterator>);
  static_assert(!RandomAccessIterator<std::list<int>::const_iterator>);
  static_assert(!RandomAccessIterator<std::list<int>::reverse_iterator>);
  static_assert(!RandomAccessIterator<std::list<int>::const_reverse_iterator>);

  static_assert(!RandomAccessIterator<std::set<int>::iterator>);
  static_assert(!RandomAccessIterator<std::set<int>::const_iterator>);
  static_assert(!RandomAccessIterator<std::set<int>::reverse_iterator>);
  static_assert(!RandomAccessIterator<std::set<int>::const_reverse_iterator>);

  static_assert(!RandomAccessIterator<std::unordered_set<int>::iterator>);
  static_assert(!RandomAccessIterator<std::unordered_set<int>::const_iterator>);

  static_assert(!RandomAccessIterator<std::map<int, int>::iterator>);
  static_assert(!RandomAccessIterator<std::map<int, int>::const_iterator>);
  static_assert(!RandomAccessIterator<std::map<int, int>::reverse_iterator>);
  static_assert(!RandomAccessIterator<std::map<int, int>::const_reverse_iterator>);

  static_assert(!RandomAccessIterator<std::unordered_map<int, int>::iterator>);
  static_assert(!RandomAccessIterator<std::unordered_map<int, int>::const_iterator>);
}

consteval auto forwardIterableTest() {
  static_assert(ForwardIterable<cds::Iterable<int>>);
  static_assert(ForwardIterable<cds::Collection<int>>);
  static_assert(ForwardIterable<cds::MutableCollection<int>>);
  static_assert(ForwardIterable<cds::List<int>>);
  static_assert(ForwardIterable<cds::Set<int>>);
  static_assert(ForwardIterable<cds::Map<int, int>>);

  static_assert(ForwardIterable<cds::Array<int>>);
  static_assert(ForwardIterable<cds::StaticArray<int, 5>>);
  static_assert(ForwardIterable<cds::List<int>>);

  static_assert(ForwardIterable<cds::HashSet<int>>);
  static_assert(ForwardIterable<cds::TreeSet<int>>);
  static_assert(ForwardIterable<cds::LinkedHashSet<int>>);

  static_assert(ForwardIterable<cds::HashMap<int, int>>);
  static_assert(ForwardIterable<cds::TreeMap<int, int>>);
  static_assert(ForwardIterable<cds::LinkedHashMap<int, int>>);

  static_assert(ForwardIterable<std::vector<int>>);
  static_assert(ForwardIterable<std::array<int, 5>>);
  static_assert(ForwardIterable<std::list<int>>);

  static_assert(ForwardIterable<std::set<int>>);
  static_assert(ForwardIterable<std::unordered_set<int>>);

  static_assert(ForwardIterable<std::map<int, int>>);
  static_assert(ForwardIterable<std::unordered_map<int, int>>);
}

consteval auto bidirectionalIterableTest() {
  static_assert(!BidirectionalIterable<cds::Iterable<int>>);
  static_assert(!BidirectionalIterable<cds::Collection<int>>);
  static_assert(!BidirectionalIterable<cds::MutableCollection<int>>);
  static_assert(BidirectionalIterable<cds::List<int>>);
  static_assert(!BidirectionalIterable<cds::Set<int>>);
  static_assert(!BidirectionalIterable<cds::Map<int, int>>);

  static_assert(BidirectionalIterable<cds::Array<int>>);
  static_assert(BidirectionalIterable<cds::StaticArray<int, 5>>);
  static_assert(BidirectionalIterable<cds::List<int>>);

  static_assert(!BidirectionalIterable<cds::HashSet<int>>);
  static_assert(BidirectionalIterable<cds::TreeSet<int>>);
  static_assert(!BidirectionalIterable<cds::LinkedHashSet<int>>);

  static_assert(!BidirectionalIterable<cds::HashMap<int, int>>);
  static_assert(BidirectionalIterable<cds::TreeMap<int, int>>);
  static_assert(!BidirectionalIterable<cds::LinkedHashMap<int, int>>);

  static_assert(BidirectionalIterable<std::vector<int>>);
  static_assert(BidirectionalIterable<std::array<int, 5>>);
  static_assert(BidirectionalIterable<std::list<int>>);

  static_assert(BidirectionalIterable<std::set<int>>);
  static_assert(!BidirectionalIterable<std::unordered_set<int>>);

  static_assert(BidirectionalIterable<std::map<int, int>>);
  static_assert(!BidirectionalIterable<std::unordered_map<int, int>>);
}

consteval auto randomAccessIterableTest() {
  static_assert(!RandomAccessIterable<cds::Iterable<int>>);
  static_assert(!RandomAccessIterable<cds::Collection<int>>);
  static_assert(!RandomAccessIterable<cds::MutableCollection<int>>);
  static_assert(!RandomAccessIterable<cds::List<int>>);
  static_assert(!RandomAccessIterable<cds::Set<int>>);
  static_assert(!RandomAccessIterable<cds::Map<int, int>>);

  static_assert(!RandomAccessIterable<cds::Array<int>>); // FIXME LV : add op +=
  static_assert(!RandomAccessIterable<cds::StaticArray<int, 5>>); // FIXME LV : add op +=
  static_assert(!RandomAccessIterable<cds::List<int>>);

  static_assert(!RandomAccessIterable<cds::HashSet<int>>);
  static_assert(!RandomAccessIterable<cds::TreeSet<int>>);
  static_assert(!RandomAccessIterable<cds::LinkedHashSet<int>>);

  static_assert(!RandomAccessIterable<cds::HashMap<int, int>>);
  static_assert(!RandomAccessIterable<cds::TreeMap<int, int>>);
  static_assert(!RandomAccessIterable<cds::LinkedHashMap<int, int>>);

  static_assert(RandomAccessIterable<std::vector<int>>);
  static_assert(RandomAccessIterable<std::array<int, 5>>);
  static_assert(!RandomAccessIterable<std::list<int>>);

  static_assert(!RandomAccessIterable<std::set<int>>);
  static_assert(!RandomAccessIterable<std::unordered_set<int>>);

  static_assert(!RandomAccessIterable<std::map<int, int>>);
  static_assert(!RandomAccessIterable<std::unordered_map<int, int>>);
}

void f() {
  isSameTest();
  dereferenceableTest();
  convertibleToTest();
  weaklyIncrementableTest();
  weaklyDecrementableTest();
  sentinelForTest();
  forwardIteratorTest();
  bidirectionalIteratorTest();
  partiallyOrderedWithTest();
  totallyOrderedTest();
  randomAccessIteratorTest();
  forwardIterableTest();
  bidirectionalIterableTest();
  randomAccessIterableTest();
}
