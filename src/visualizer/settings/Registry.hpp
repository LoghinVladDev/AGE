//
// Created by Vlad-Andrei Loghin on 24.09.23.
//

#pragma once

#include <CDS/threading/Atomic>
#include <CDS/threading/Mutex>
#include <CDS/util/JSON>

#include <lang/string/StringRef.hpp>
#include <lang/thread/AsyncRunner.hpp>
#include <logging/Logger.hpp>

namespace age::visualizer::settings {
class Registry {
public:
  class Transaction {
  public:
    using Transformer = cds::Function<void(cds::json::JsonElement&)>;

    explicit Transaction(Registry& registry) noexcept : _registry(registry) {}

    template <typename... Groups> explicit Transaction(Registry& registry, Groups&&... preloadGroups) noexcept(false) :
        _registry(registry) {
      preload(std::forward<Groups>(preloadGroups)...);
    }

    Transaction(Transaction&& transaction) noexcept = default;
    ~Transaction() noexcept { _registry.commit(std::move(*this)); }

    [[nodiscard]] constexpr auto registry() noexcept -> Registry& { return _registry; }
    auto get(StringRef key) noexcept(false) -> cds::json::JsonElement&;
    template <typename T> auto getOrDefault(StringRef key, T&& value) noexcept(false) -> cds::json::JsonElement& {
      return getOrDefaultImpl(key, {std::forward<T>(value)});
    }

    template <typename T> auto operator[](StringRef key) noexcept(false) -> cds::json::JsonElement& {
      return getOrDefaultImpl(key, {T {}});
    }

    auto transform(StringRef key, Transformer const& func) noexcept(false) -> Transaction&;
    template <typename T>
    auto defaultAndTransform(StringRef key, T&& defaultValue, Transformer const& func) noexcept(false) -> Transaction& {
      return defaultAndTransformImpl(key, {std::forward<T>(defaultValue)}, func);
    }

    template <typename T> auto storeIfPresent(StringRef key, T&& value) noexcept(false) -> Transaction& {
      return storeIfPresentImpl(key, {std::forward<T>(value)});
    }

    template <typename T> auto storeIfAbsent(StringRef key, T&& value) noexcept(false) -> Transaction& {
      return storeIfAbsentImpl(key, {std::forward<T>(value)});
    }

    template <typename T> auto storeOrOverwrite(StringRef key, T&& value) noexcept(false) -> Transaction& {
      return storeOrOverwriteImpl(key, {std::forward<T>(value)});
    }

    auto remove(StringRef key) noexcept(false) -> Transaction&;

  private:
    auto getOrDefaultImpl(StringRef key, cds::json::JsonElement&& value) noexcept(false) -> cds::json::JsonElement&;
    auto defaultAndTransformImpl(StringRef key, cds::json::JsonElement&& value, Transformer const& func) noexcept(false)
        -> Transaction&;
    auto storeIfPresentImpl(StringRef key, cds::json::JsonElement&& value) noexcept(false) -> Transaction&;
    auto storeIfAbsentImpl(StringRef key, cds::json::JsonElement&& value) noexcept(false) -> Transaction&;
    auto storeOrOverwriteImpl(StringRef key, cds::json::JsonElement&& value) noexcept(false) -> Transaction&;
    auto findLocal(StringRef key) noexcept(false) -> cds::json::JsonElement*;
    auto findFetch(StringRef key) noexcept(false) -> cds::json::JsonElement*;

    auto preload() const noexcept -> void {
      (void) this;
      // empty on purpose
    }

    auto preload(StringRef group) noexcept(false) -> void;
    template <typename F, typename... R> auto preload(F&& first, R&&... remaining) noexcept(false) -> void {
      preload(std::forward<F>(first));
      preload(std::forward<R>(remaining)...);
    }

    Registry& _registry;
    cds::LinkedHashMap<cds::String, cds::json::JsonElement> _values;
    cds::Array<cds::String> _targetedRemoveKeys;

    static inline auto& logger = Logger::get("visualizer.settings.Registry.Transaction");

#ifndef NDEBUG
    struct Id {
      static inline cds::Atomic _tIdCtr = 0;
      int _transactionId = ++_tIdCtr;
    };

    Id _id;

    [[nodiscard]] constexpr auto id() const noexcept -> int { return _id._transactionId; }
#else
    [[nodiscard]] constexpr auto id() const noexcept -> int { return 0; }
#endif
  };

  Registry(StringRef path) noexcept(false);
  ~Registry() noexcept;

  [[nodiscard]] auto transaction() noexcept { return Transaction {*this}; }
  template <typename... Groups> [[nodiscard]] auto transaction(Groups&&... groups) noexcept {
    return Transaction {std::forward<Groups>(groups)...};
  }

  auto startProcessPendingTransactions() noexcept(false) -> void;
  auto waitForProcessingTransactions() noexcept(false) -> void;

  [[nodiscard]] auto getInt(StringRef key) const noexcept(false) -> int { return fetch(key).getInt(); }
  [[nodiscard]] auto getLong(StringRef key) const noexcept(false) -> long { return fetch(key).getLong(); }
  [[nodiscard]] auto getFloat(StringRef key) const noexcept(false) -> float { return fetch(key).getFloat(); }
  [[nodiscard]] auto getDouble(StringRef key) const noexcept(false) -> double { return fetch(key).getDouble(); }
  [[nodiscard]] auto getString(StringRef key) const noexcept(false) -> cds::String { return fetch(key).getString(); }

  [[nodiscard]] auto getArray(StringRef key) const noexcept(false) -> cds::json::JsonArray {
    return fetch(key).getArray();
  }

  [[nodiscard]] auto getJson(StringRef key) const noexcept(false) -> cds::json::JsonObject {
    return fetch(key).getJson();
  }

private:
  auto commit(Transaction&& transaction) noexcept(false) -> void;
  auto fetch(StringRef key) const noexcept(false) -> cds::json::JsonElement;
  auto awaitInit() const noexcept { _loader->await(); }

  auto wakeProcessor() noexcept(false) -> void;

  static auto loaderThreadFn(Registry* pCallee) noexcept -> void;
  static auto transactionProcessorFn() noexcept -> void;

  cds::json::JsonObject _group1;
  cds::json::JsonObject _group2;
  cds::Atomic<cds::json::JsonObject*> _active {nullptr};
  cds::Atomic<cds::json::JsonObject*> _next {nullptr};

  cds::UniquePointer<AsyncRunner<void, Registry*>> _loader {
      cds::makeUnique<AsyncRunner<void, Registry*>>(&loaderThreadFn)};
  AsyncRunner<void> _transactionProcessor {&transactionProcessorFn};

  cds::Mutex _swapLock;
  struct {
    int processTriggerCount = 16;
    cds::Array<Transaction> data;
    cds::Mutex mtx;
  } _transactions; // To be replaced with LockFreeQueue

  static inline auto& logger = Logger::get("visualizer.settings.Registry");
};
} // namespace age::visualizer::settings
