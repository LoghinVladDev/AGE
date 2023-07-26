//
// Created by Vlad-Andrei Loghin on 18.06.23.
//

#pragma once

#include <CDS/memory/UniquePointer>

#include <QLayout>
#include <QWidget>

namespace age::visualizer {
class VisualizerWindow : public QWidget {
  Q_OBJECT
public:
  explicit VisualizerWindow(QWidget* pParent = nullptr) noexcept;
  ~VisualizerWindow() noexcept override = default;

private:
  cds::UniquePointer<QLayout> _primaryLayout {nullptr};
};
} // namespace age::visualizer
