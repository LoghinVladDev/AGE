//
// Created by stefan on 8/7/23.
//

#pragma once

#include <QPainter>
#include <QWidget>

namespace age::visualizer {
class Vertex;
class VertexMenu : public QWidget {
  Q_OBJECT
public:
  explicit VertexMenu(QWidget* pParent) noexcept;

public slots:
  void popup(age::visualizer::Vertex* pVertex, QPoint const& qPoint);

protected:
  auto paintEvent(QPaintEvent* pEvent) -> void override;
};
} // namespace age::visualizer
