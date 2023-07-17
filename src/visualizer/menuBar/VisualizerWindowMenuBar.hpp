//
// Created by Vlad-Andrei Loghin on 10.07.23.
//

#pragma once

#include <QMenuBar>

#include <CDS/Array>
#include <CDS/memory/UniquePointer>

namespace age::visualizer {
class VisualizerWindowMenuBar : public QMenuBar {
  Q_OBJECT
public:
  explicit VisualizerWindowMenuBar(QWidget* parent = nullptr) noexcept;

private:
  cds::Array<cds::UniquePointer<QObject>> _storage;
};
} // namespace age::visualizer
