//
// Created by Vlad-Andrei Loghin on 18.06.23.
//

#include "SettingsRegistry.hpp"
#include <CDS/filesystem/Path>
#include <CDS/threading/Thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <tuple>

namespace {
using namespace cds;
using namespace cds::filesystem;
using namespace cds::json;
using namespace age;
using namespace age::visualizer::settings;

using std::atomic_flag;
using std::condition_variable;
using std::lock_guard;
using std::mutex;
using std::tuple;
using std::unique_lock;

auto sub(StringRef& key) noexcept -> StringRef {
  auto dotPos = key.find('.');
  if (dotPos == StringRef::npos) {
    return cds::exchange(key, "");
  }

  auto rVal = key.sub(0, dotPos);
  key = key.dropFront(dotPos + 1);
  return rVal;
}

auto get(auto& json, StringRef key) noexcept(false) -> auto& {
  auto current = &json;
  auto subKey = sub(key);
  while (key) {
    current = &current->getJson(subKey);
    subKey = sub(key);
  }

  return current->get(subKey);
}

template <typename T, bool = cds::meta::IsVoid<T>::value> class AsyncResultContainer {
public:
  template <typename F, typename... A> auto awaitResult(F&& fn, A&&... a) { fn(std::forward<A>(a)...); }
};

template <typename T> class AsyncResultContainer<T, false> {
public:
  template <typename F, typename... A> auto awaitResult(F&& fn, A&&... a) { value = fn(std::forward<A>(a)...); }

private:
  T value;
};

template <typename T, typename R, bool = cds::meta::IsVoid<R>::value> class AwaitWrapper {};

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

template <typename Result, typename... Args> class Async : public AwaitWrapper<Async<Result, Args...>, Result> {
public:
  using Function = cds::Function<Result(Args...)>;

  Async() noexcept = delete;
  Async(Async const&) noexcept = delete;
  Async(Async&&) noexcept = delete;
  ~Async() noexcept { join(); }

  auto operator=(Async const&) noexcept = delete;
  auto operator=(Async&&) noexcept = delete;

  template <typename F> explicit(false) Async(F&& function) noexcept(false) : _fn(std::forward<F>(function)) {}

  auto reset() {
    _ended.clear(std::memory_order_release);
    std::get<2>(_sync) = false;
    std::get<3>(_sync) = false;
  }

  template <typename... A> auto trigger(A&&... args) noexcept(false) -> void {
    reset();
    _executer = new Runnable([this, args...]() mutable {
      auto& [mutex, condition, startStatus, endStatus] = _sync;
      unique_lock lock(mutex);
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
      lock_guard _(mutex);
      startStatus = true;
    };

    prepareNotify();
    condition.notify_one();
  }

  [[nodiscard]] auto notStarted() const noexcept -> bool { return !std::get<2>(_sync); }

private:
  friend class AwaitWrapper<Async<Result, Args...>, Result>;
  auto join() noexcept {
    if (!_started.test(std::memory_order_acquire)) {
      return;
    }

    auto& [mutex, condition, startStatus, endStatus] = _sync;
    auto prepareStop = [&mutex, &condition, &endStatus] {
      unique_lock lock(mutex);
      condition.wait(lock, [&endStatus] { return endStatus; });
    };

    if (_ended.test_and_set()) {
      return;
    }

    prepareStop();
    _executer->join();
    _executer.release();
  }

  Function _fn {nullptr};
  [[no_unique_address]] AsyncResultContainer<Result> _result;
  UniquePointer<Thread> _executer {nullptr};
  tuple<mutex, condition_variable, bool, bool> _sync;
  atomic_flag _started;
  atomic_flag _ended;
};

auto const loader = cds::makeUnique<Async<void, JsonObject*, JsonObject*>>([](JsonObject* main, JsonObject* copy) {
  main->put("testStr", "test");
  main->put("testInt", 0);
  main->put("testFloat", 0.0f);
  main->put("testBool", false);
  main->put("testArray", JsonArray());
  main->put("testJson", JsonObject().put("testStr", "testSub"));
  *copy = *main;
});

auto const saver = cds::makeUnique<Async<void, Path, JsonObject const*>>(
    [](Path path, JsonObject const* json) { std::cout << "Trigger save of json to '" << path << "'\n"; });

auto convertToPath(StringRef key) noexcept -> String {
  String path = "./";
  while (key) {
    auto dotPos = key.find('.');
    if (dotPos == StringRef::npos) {
      dotPos = key.size();
    }

    path += StringView(key.takeFront(dotPos));
    key = key.dropFront(dotPos + 1);
    if (key) {
      path += "/";
    }
  }
  return path + ".json";
}
} // namespace

auto Registry::sub(StringRef& key) noexcept -> StringRef { return ::sub(key); }

auto Registry::active() noexcept(false) -> Registry& {
  if (loader->notStarted()) {
    triggerLoad();
  }
  loader->await();
  return *_active;
}

auto Registry::triggerLoad() noexcept(false) -> void {
  if (!_active) {
    _active = cds::makeUnique<Registry>(Token {});
    _stored = cds::makeUnique<Registry>(Token {});
  }

  loader->trigger(&_active->_contents, &_stored->_contents);
}

auto Registry::reset(StringRef key) noexcept(false) -> void {
  auto* lJson = &_active->_contents;
  auto* rJson = &_stored->_contents;

  if (!key) {
    *lJson = *rJson;
    return;
  }

  auto subKey = sub(key);
  while (key) {
    lJson = &lJson->getJson(subKey);
    rJson = &rJson->getJson(subKey);
    subKey = sub(key);
  }

  lJson->get(subKey) = rJson->get(subKey);
}

auto Registry::replaceIfMissing(JsonObject* pJson, StringRef key, bool overwriteType) noexcept -> void {
  if (auto jsonIt = pJson->find(key); jsonIt == pJson->end()) {
    pJson->put(key, JsonObject());
  } else if (overwriteType || !jsonIt->value().isJson()) {
    jsonIt->value() = JsonObject();
  }
}

auto Registry::save(StringRef key) noexcept(false) -> void {
  auto lJson = &_stored->_contents;
  auto rJson = &_active->_contents;
  String savePath = key ? convertToPath(key) : defaultPath;

  if (key) {
    auto subKey = sub(key);
    while (key) {
      replaceIfMissing(lJson, subKey, true);

      lJson = &lJson->getJson(subKey);
      rJson = &rJson->getJson(subKey);
      subKey = sub(key);
    }
    saver->await();
    replaceIfMissing(lJson, subKey);
    lJson->get(subKey) = rJson->get(subKey);
  } else {
    saver->await();
    *lJson = *rJson;
  }

  saver->trigger(savePath, lJson);
}

auto Registry::getInt(StringRef key) const noexcept(false) -> int { return get(_contents, key).getInt(); }
auto Registry::getLong(StringRef key) const noexcept(false) -> long { return get(_contents, key).getLong(); }
auto Registry::getFloat(StringRef key) const noexcept(false) -> float { return get(_contents, key).getFloat(); }
auto Registry::getDouble(StringRef key) const noexcept(false) -> double { return get(_contents, key).getDouble(); }

auto Registry::getString(StringRef key) const noexcept(false) -> String const& {
  return get(_contents, key).getString();
}

auto Registry::getArray(StringRef key) const noexcept(false) -> JsonArray const& {
  return get(_contents, key).getArray();
}

auto Registry::getJson(StringRef key) const noexcept(false) -> JsonObject const& {
  return get(_contents, key).getJson();
}

auto Registry::getString(StringRef key) noexcept(false) -> String& { return get(_contents, key).getString(); }
auto Registry::getArray(StringRef key) noexcept(false) -> JsonArray& { return get(_contents, key).getArray(); }
auto Registry::getJson(StringRef key) noexcept(false) -> JsonObject& { return get(_contents, key).getJson(); }
