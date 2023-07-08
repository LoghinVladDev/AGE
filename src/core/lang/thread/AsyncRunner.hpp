//
// Created by Vlad-Andrei Loghin on 06.07.23.
//

#ifndef AGE_ASYNC_RUNNER_HPP
#define AGE_ASYNC_RUNNER_HPP

#include <CDS/Function>
#include <CDS/memory/UniquePointer>
#include <CDS/meta/Base>
#include <CDS/threading/Thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <tuple>

namespace age {
namespace meta {
template <typename T, typename R, bool = cds::meta::IsVoid<R>::value> class AwaitWrapper {};

template <typename T, bool = cds::meta::IsVoid<T>::value> class AsyncResultContainer {
public:
  template <typename F, typename... A> auto awaitResult(F&& fn, A&&... a) { fn(std::forward<A>(a)...); }
};

template <typename T> class AsyncResultContainer<T, false> {
public:
  template <typename F, typename... A> auto awaitResult(F&& fn, A&&... a) { value = fn(std::forward<A>(a)...); }

private:
  template <typename, typename, bool> friend class AwaitWrapper;
  T value;
};

template <typename T, typename R> class AwaitWrapper<T, R, true> {
public:
  auto await() noexcept -> void { static_cast<T*>(this)->join(); }
};

template <typename T, typename R> class AwaitWrapper<T, R, false> {
public:
  auto await() noexcept -> R {
    static_cast<T*>(this)->join();
    return static_cast<T*>(this)->_result.value;
  }
};
} // namespace meta

template <typename Result, typename... Args> class AsyncRunner :
    public meta::AwaitWrapper<AsyncRunner<Result, Args...>, Result> {
public:
  using Function = cds::Function<Result(Args...)>;

  AsyncRunner() noexcept = delete;
  AsyncRunner(AsyncRunner const&) noexcept = delete;
  AsyncRunner(AsyncRunner&&) noexcept = delete;
  ~AsyncRunner() noexcept { join(); }

  auto operator=(AsyncRunner const&) noexcept = delete;
  auto operator=(AsyncRunner&&) noexcept = delete;

  template <typename F> explicit(false) AsyncRunner(F&& function) noexcept(false) : _fn(std::forward<F>(function)) {}

  template <typename... A> auto trigger(A&&... args) noexcept(false);
  [[nodiscard]] auto notStarted() const noexcept { return !std::get<2>(_sync); }

private:
  friend class meta::AwaitWrapper<AsyncRunner<Result, Args...>, Result>;
  auto join() noexcept;
  auto reset();

  Function _fn {nullptr};
  [[no_unique_address]] meta::AsyncResultContainer<Result> _result;
  cds::UniquePointer<cds::Thread> _executer {nullptr};
  std::tuple<std::mutex, std::condition_variable, bool, bool> _sync;
  std::atomic_flag _started;
  std::atomic_flag _ended;
};

template <typename Result, typename... Args> auto AsyncRunner<Result, Args...>::reset() {
  _ended.clear(std::memory_order_release);
  std::get<2>(_sync) = false;
  std::get<3>(_sync) = false;
}

template <typename Result, typename... Args> auto AsyncRunner<Result, Args...>::join() noexcept {
  if (!_started.test(std::memory_order_acquire)) {
    return;
  }

  auto& [mutex, condition, startStatus, endStatus] = _sync;
  auto prepareStop = [&mutex, &condition, &endStatus] {
    std::unique_lock lock(mutex);
    condition.wait(lock, [&endStatus] { return endStatus; });
  };

  if (_ended.test_and_set()) {
    return;
  }

  prepareStop();
  _executer->join();
  _executer.release();
}

template <typename Result, typename... Args> template <typename... A>
auto AsyncRunner<Result, Args...>::trigger(A&&... args) noexcept(false) {
  reset();
  _executer = new cds::Runnable([this, args...]() mutable {
    auto& [mutex, condition, startStatus, endStatus] = _sync;
    std::unique_lock lock(mutex);
    condition.wait(lock, [&startStatus] { return startStatus; });
    _result.awaitResult(_fn, args...);
    endStatus = true;
    lock.unlock();
    condition.notify_one();
  });
  (void) _started.test_and_set(std::memory_order_release);
  _executer->start();

  auto& [mutex, condition, startStatus, endStatus] = _sync;
  auto prepareNotify = [&mutex, &startStatus] {
    std::lock_guard _(mutex);
    startStatus = true;
  };

  prepareNotify();
  condition.notify_one();
}
} // namespace age

#endif // AGE_ASYNC_RUNNER_HPP
