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
    co_yield base + (index) + "__";
    ++index;
  }
}

auto separatorGenerator = separatorGeneratorFormula();

auto const defaultConfig = JsonObject()
                               .put("File",
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
                                        .put("About...", actionId(ActionBinding::AB_about)));

auto loadByUnderlying(auto* parent, auto const& name, JsonObject const& data) {
  // empty for now
}

auto loadByUnderlying(auto* parent, auto const& name, auto const& data) {
  if (data.isJson()) {
    return loadByUnderlying(parent, name, data.getJson());
  }
}

auto loadMenuConfig(QMenuBar* menu) -> void {
  auto& config = registry().getJsonOr("window.menubar", defaultConfig);
  for (auto const& primaryMenuData : config) {
    auto const& primaryMenuName = primaryMenuData.key();
    auto const& primaryMenuItems = primaryMenuData.value();
    loadByUnderlying(menu, primaryMenuName, primaryMenuItems);
  }
}
} // namespace

VisualizerWindowMenuBar::VisualizerWindowMenuBar(QWidget* parent) noexcept : QMenuBar(parent) { loadMenuConfig(this); }
