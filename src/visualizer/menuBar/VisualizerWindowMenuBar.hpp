//
// Created by Vlad-Andrei Loghin on 10.07.23.
//

#pragma once

#include <QMenuBar>

namespace age::visualizer {
class VisualizerWindowMenuBar : public QMenuBar {
  Q_OBJECT
public:
  explicit VisualizerWindowMenuBar(QWidget* parent = nullptr) noexcept;
};
} // namespace age::visualizer
