//
// Created by Vlad-Andrei Loghin on 17.06.23.
//

#include <QApplication>
#include <QScreen>
#include <menuBar/VisualizerWindowMenuBar.hpp>
#include <settings/SettingsRegistry.hpp>
#include <window/VisualizerWindow.hpp>

namespace {
using age::visualizer::VisualizerWindow;
using age::visualizer::VisualizerWindowMenuBar;
using age::visualizer::settings::Registry;
using age::visualizer::settings::registry;
using cds::UniquePointer;

auto startingGeometry() {
  auto screenSize = QApplication::primaryScreen()->availableSize();
  auto width = registry().getIntOr("session.baseWindow.width", std::min(1280, screenSize.width()));
  auto height = registry().getIntOr("session.baseWindow.height", std::min(720, screenSize.height()));
  auto defaultX = (screenSize.width() - width) / 2;
  auto defaultY = (screenSize.height() - height) / 2;
  auto x = registry().getIntOr("session.baseWindow.x", defaultX);
  auto y = registry().getIntOr("session.baseWindow.y", defaultY);
  return std::make_tuple(x, y, width, height);
}

auto saveSession() { registry().save("session"); }

class WindowWrapper : public VisualizerWindow {
public:
  WindowWrapper() : VisualizerWindow() { layout()->setMenuBar(_menuBar); }

  auto closeEvent(QCloseEvent* event) -> void override {
    QWidget::closeEvent(event);
    registry().replace("session.baseWindow.x", pos().x());
    registry().replace("session.baseWindow.y", pos().y());
    registry().replace("session.baseWindow.width", size().width());
    registry().replace("session.baseWindow.height", size().height());
    saveSession();
  }

private:
  UniquePointer<QWidget> _menuBar {cds::makeUnique<VisualizerWindowMenuBar>(this)};
};
} // namespace

int main(int argc, char** argv) {
  Registry::triggerLoad();
  ::QApplication app(argc, argv);
  WindowWrapper w;
  auto [x, y, width, height] = startingGeometry();
  w.resize(width, height);
  w.move(x, y);
  w.show();
  return ::QApplication::exec();
}
