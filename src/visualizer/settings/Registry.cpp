//
// Created by Vlad-Andrei Loghin on 24.09.23.
//

#include "Registry.hpp"

#include <CDS/LinkedList>
#include <CDS/threading/Lock>

namespace {
using namespace age;
using namespace age::visualizer::settings;
using namespace cds;
using namespace cds::json;
using enum Logger::Level;
} // namespace

auto Registry::Transaction::findLocal(StringRef key) noexcept(false) -> JsonElement* {
  if (auto const localIt = _values.find(key); localIt != _values.end()) {
    return &localIt->value();
  }

  return nullptr;
}

auto Registry::Transaction::findFetch(StringRef key) noexcept(false) -> JsonElement* {
  if (auto const pValue = findLocal(key)) {
    return pValue;
  }

  return &_values.emplace(key, _registry.fetch(key)).value();
}

auto Registry::Transaction::get(StringRef key) noexcept(false) -> JsonElement& {
  if (auto const pValue = findFetch(key)) {
    return *pValue;
  }

  logger(Warning) << "Registry does not contain key '" << key << "', requested in Transaction '" << id() << "'";
  throw cds::KeyException(key);
}

auto Registry::Transaction::transform(StringRef key, Transformer const& func) noexcept(false) -> Transaction& {
  JsonElement* pElement;
  if (auto const pValue = findFetch(key)) {
    pElement = pValue;
  } else {
    pElement = &_values.emplace(key, JsonElement {0}).value();
  }

  func(*pElement);
  return *this;
}

auto Registry::Transaction::getOrDefaultImpl(StringRef key, JsonElement&& value) noexcept(false) -> JsonElement& {
  if (auto const pValue = findFetch(key)) {
    return *pValue;
  }

  return _values.emplace(key, std::move(value)).value();
}

auto Registry::Transaction::remove(StringRef key) noexcept(false) -> Transaction& {
  _targetedRemoveKeys.emplace(key);
  return *this;
}

auto Registry::Transaction::defaultAndTransformImpl(StringRef key, JsonElement&& value,
                                                    Transformer const& func) noexcept(false) -> Transaction& {
  func(getOrDefaultImpl(key, std::move(value)));
  return *this;
}

auto Registry::Transaction::storeIfPresentImpl(StringRef key, JsonElement&& value) noexcept(false) -> Transaction& {
  if (auto const pValue = findFetch(key)) {
    *pValue = std::move(value);
  }

  return *this;
}

auto Registry::Transaction::storeIfAbsentImpl(StringRef key, JsonElement&& value) noexcept(false) -> Transaction& {
  if (auto const pValue = findFetch(key)) {
    return *this;
  }

  _values.emplace(key, std::move(value));
  return *this;
}

auto Registry::Transaction::storeOrOverwriteImpl(StringRef key, JsonElement&& value) noexcept(false) -> Transaction& {
  if (auto const pValue = findFetch(key)) {
    *pValue = std::move(value);
  } else {
    _values.emplace(key, std::move(value));
  }

  return *this;
}

auto Registry::Transaction::preload(StringRef key) noexcept(false) -> void {
  try {
    auto checkedForKeys = cds::listOf(Tuple {String(key), _registry.fetch(key)});
    while (checkedForKeys) {
      auto front = checkedForKeys.front();
      checkedForKeys.popFront();
      auto& object = _values.emplace(front.get<0>(), std::move(front.get<1>())).value();
      if (object.isJson()) {
        for (auto const& entry : object.getJson()) {
          checkedForKeys.emplaceBack(front.get<0>() + "." + entry.key(), entry.value());
        }
      }
    }
  } catch (cds::KeyException const& keyException) {
    logger(Warning) << "Registry does not contain preload key '" << key << "', requested in Transaction '" << id()
                    << "'. Exception: " << keyException;
  }
}

Registry::Registry(StringRef path) noexcept(false) { _loader->trigger(this); }

auto Registry::startProcessPendingTransactions() noexcept(false) -> void {
  Lock lock(_transactions.mtx);
  wakeProcessor();
}

auto Registry::waitForProcessingTransactions() noexcept(false) -> void { _transactionProcessor.await(); }

auto Registry::commit(Transaction&& transaction) noexcept(false) -> void {
  Lock lock(_transactions.mtx);
  _transactions.data.pushBack(std::move(transaction));

  if (_transactions.data.size() >= _transactions.processTriggerCount) {
    wakeProcessor();
  }
}

auto Registry::wakeProcessor() noexcept(false) -> void { _transactionProcessor.trigger(); }

auto Registry::fetch(StringRef key) const noexcept(false) -> JsonElement {
  awaitInit();
  // not enough, need "group.group" decompose. TODO
  return _active.get()->get(key);
}

auto Registry::loaderThreadFn(Registry* pCallee) noexcept -> void {
  // TODO: disk load logic
  pCallee->_active = &pCallee->_group1;
  pCallee->_next = &pCallee->_group2;
}

auto Registry::transactionProcessorFn() noexcept -> void {
  // TODO: process queue of transactions
}
