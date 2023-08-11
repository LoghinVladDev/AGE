//
// Created by Vlad-Andrei Loghin on 07.07.23.
//

#pragma once
#include <coroutine>
#include <exception>

#include <CDS/meta/TypeTraits>
#include <lang/iter/Sentinel.hpp>

namespace age {
namespace meta {
template <typename T, bool = cds::meta::IsFundamental<T>::value> struct MovableIterableReturn {};

template <typename T> struct MovableIterableReturn<T, true> {
  using Type = cds::meta::Decay<T>;
};

template <typename T> struct MovableIterableReturn<T, false> {
  using Type = cds::meta::AddLValueReference<cds::meta::Decay<T>>;
};
} // namespace meta

template <typename T> class Generator {
public:
  class Iterator {
  public:
    explicit Iterator(Generator<T>&& generator) noexcept : _generator(std::move(generator)) {}
    auto operator++() -> Iterator& {
      _generator._acquired = false;
      _generator.acquire();
      return *this;
    }

    auto operator*() -> meta::MovableIterableReturn<T>::Type { return _generator._handle.promise()._value; }

    auto operator!=(DefaultSentinel) -> bool { return !_generator.empty(); }

  private:
    Generator _generator;
  };

  struct promise_type {
    auto get_return_object() noexcept -> Generator {
      return Generator {std::coroutine_handle<promise_type>::from_promise(*this)};
    }

    [[nodiscard]] auto initial_suspend() const noexcept -> std::suspend_always { return {}; }
    [[nodiscard]] auto final_suspend() const noexcept -> std::suspend_always { return {}; }
    void unhandled_exception() noexcept { _exception = std::current_exception(); }

    template <typename From>
      requires std::convertible_to<From, T>
    auto yield_value(From&& from) -> std::suspend_always {
      _value = std::forward<From>(from);
      return {};
    }

    auto return_void() const noexcept -> void {
      // empty on purpose
    }

    T _value;
    std::exception_ptr _exception;
  };

  explicit Generator(std::coroutine_handle<promise_type> handle) noexcept : _handle(handle) {}
  ~Generator() noexcept {
    if (_handle) {
      _handle.destroy();
    }
  }

  [[nodiscard]] auto get() -> T {
    acquire();
    _acquired = false;
    return std::move(_handle.promise()._value);
  }

  [[nodiscard]] auto empty() -> bool {
    acquire();
    return _handle.done();
  }

  [[nodiscard]] auto begin() noexcept -> Iterator {
    Iterator rVal(std::move(*this));
    _handle = nullptr;
    return rVal;
  }

  [[nodiscard]] auto end() const noexcept -> DefaultSentinel { return {}; }

private:
  auto acquire() -> void {
    if (!_acquired) {
      _handle();
      if (_handle.promise()._exception) {
        std::rethrow_exception(_handle.promise()._exception);
      }
      _acquired = true;
    }
  }

  std::coroutine_handle<promise_type> _handle;
  bool _acquired = false;
};

} // namespace age
