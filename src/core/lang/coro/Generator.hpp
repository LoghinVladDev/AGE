//
// Created by Vlad-Andrei Loghin on 07.07.23.
//

#ifndef AGE_GENERATOR_HPP
#define AGE_GENERATOR_HPP

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
    auto operator++() noexcept -> Iterator& {
      _generator.acquire();
      return *this;
    }

    auto operator*() noexcept -> meta::MovableIterableReturn<T>::Type {
      _generator.acquire();
      _generator._acquired = false;
      return _generator._handle.promise()._value;
    }

    auto operator!=(DefaultSentinel) noexcept -> bool { return !_generator.empty(); }

  private:
    Generator _generator;
  };

  struct promise_type {
    auto get_return_object() noexcept -> Generator {
      return Generator {std::coroutine_handle<promise_type>::from_promise(*this)};
    }

    auto initial_suspend() noexcept -> std::suspend_always { return {}; }
    auto final_suspend() noexcept -> std::suspend_always { return {}; }
    void unhandled_exception() noexcept { _exception = std::current_exception(); }

    template <typename From>
      requires std::convertible_to<From, T>
    auto yield_value(From&& from) -> std::suspend_always {
      _value = std::forward<From>(from);
      return {};
    }

    auto return_void() noexcept -> void {}

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

  [[nodiscard]] auto end() noexcept -> DefaultSentinel { return {}; }

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

#endif // AGE_GENERATOR_HPP