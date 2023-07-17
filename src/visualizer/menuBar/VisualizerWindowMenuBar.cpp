//
// Created by Vlad-Andrei Loghin on 10.07.23.
//

#include "VisualizerWindowMenuBar.hpp"
#include <binding/Action.hpp>
#include <lang/coro/Generator.hpp>
#include <settings/SettingsRegistry.hpp>

namespace {
using namespace age;
using namespace age::visualizer;
using namespace age::visualizer::settings;
using namespace age::visualizer::meta;
using namespace cds;
using namespace cds::json;

auto separatorGeneratorFormula() -> Generator<String> {
  String base = "__separator_";
  int index = 0;
  while (true) {
    co_yield base + index + "__";
    ++index;
  }
}

auto isSeparator(StringRef key) {
  return key.startsWith("__separator_");
}

auto separatorGenerator = separatorGeneratorFormula();

auto const defaultConfigSet = JsonArray()
                               .pushBack(
                                   JsonObject().put(
                                       "File",
                                      JsonObject()
                                        .put("New",
                                             JsonObject()
                                                 .put("Project...", actionId(ActionBinding::AB_newProject))
                                                 .put("File...", actionId(ActionBinding::AB_newFile))
                                                 .put("Open...", actionId(ActionBinding::AB_openFile)))
                                        .put(separatorGenerator.get(), "")
                                        .put("Save", actionId(ActionBinding::AB_saveFile)))
                               .put("Help",
                                    JsonObject()
                                        .put("Find Action...", actionId(ActionBinding::AB_findAction))
                                        .put("About...", actionId(ActionBinding::AB_about))));

auto loadByUnderlying(auto& parent, auto const& name, auto const& data, auto& itemStorage) -> void;

auto loadByUnderlying(auto& parent, auto const& name, JsonObject const& data, auto& itemStorage) -> void {
  auto menu = makeUnique<QMenu>(name.cStr(), parent);
  for (auto const& menuData : data) {
    auto const& name = menuData.key();
    auto const& items = menuData.value();
    loadByUnderlying(menu, name, items, itemStorage);
  }
  parent->addMenu(menu);
  itemStorage.pushBack(std::move(menu));
}

auto loadByUnderlying(auto& parent, auto const& name, long long data, auto& itemStorage) -> void {
  parent->addAction(static_cast<QAction*>(itemStorage.pushBack(cds::makeUnique<QAction>(name.cStr(), parent)).get()));
}

auto loadByUnderlying(auto& parent, auto const& name, auto const& data, auto& itemStorage) -> void {
  if (data.isJson()) {
    return loadByUnderlying(parent, name, data.getJson(), itemStorage);
  }

  if (data.isLong()) {
    return loadByUnderlying(parent, name, data.getLong(), itemStorage);
  }

  if (data.isString() && isSeparator(name)) {
    parent->addSeparator();
  }
}

auto loadMenuConfig(QMenuBar* menu, auto& itemStorage) -> void {
  registry().markAsIntegral("window.menubar");
  auto& configs = registry().getArrayOr("window.menubar", defaultConfigSet);
  for (auto const& primaryMenuData : configs[0u].getJson()) {
    auto const& primaryMenuName = primaryMenuData.key();
    auto const& primaryMenuItems = primaryMenuData.value();
    loadByUnderlying(menu, primaryMenuName, primaryMenuItems, itemStorage);
  }
}
} // namespace

VisualizerWindowMenuBar::VisualizerWindowMenuBar(QWidget* parent) noexcept : QMenuBar(parent) {
  loadMenuConfig(this, _storage);
}
