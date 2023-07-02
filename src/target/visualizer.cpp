//
// Created by Vlad-Andrei Loghin on 17.06.23.
//

#include <QApplication>
#include <settings/SettingsRegistry.hpp>
#include <window/VisualizerWindow.hpp>

namespace {
using age::visualizer::VisualizerWindow;
using age::visualizer::settings::Registry;
} // namespace

int main(int argc, char** argv) {
  Registry::triggerLoad();
  ::QApplication app(argc, argv);
  VisualizerWindow w;
  w.show();
  return ::QApplication::exec();
}
