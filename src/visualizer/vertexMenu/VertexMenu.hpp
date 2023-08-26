//
// Created by stefan on 8/7/23.
//

#pragma once

#include <CDS/memory/UniquePointer>
#include <QAction>
#include <QMenu>
#include <QPainter>

namespace age::visualizer {
class VertexMenu : public QMenu {
  Q_OBJECT
public:
  explicit VertexMenu(QWidget* pParent) noexcept;
};
} // namespace age::visualizer
