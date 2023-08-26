//
// Created by stefan on 21.07.2023.
//

#pragma once
#include <QMouseEvent>
#include <QPainter>
#include <QWidget>

namespace age::visualizer {
class Vertex : public QWidget {
  Q_OBJECT
public:
  explicit Vertex(QPoint const& point, QWidget* pParent) noexcept;
  explicit Vertex(int x, int y, QWidget* pParent) noexcept;
signals:
  auto rightClickPressed(QPoint const& qPoint) -> void;

protected:
  auto mousePressEvent(QMouseEvent* pEvent) -> void override;
  auto mouseMoveEvent(QMouseEvent* pEvent) -> void override;
  auto mouseReleaseEvent(QMouseEvent* pEvent) -> void override;
  auto paintEvent(QPaintEvent* pEvent) -> void override;

private:
  auto handleLeftClickEvent(QMouseEvent* pEvent) -> void;
  static constexpr int DRAWN_VERTEX_DIAMETER = 30;
  static constexpr int WIDGET_SIZE = 40;
  static constexpr Qt::GlobalColor DEFAULT_VERTEX_COLOR = Qt::blue;
  static constexpr Qt::GlobalColor SELECTED_VERTEX_COLOR = Qt::red;
  bool _selected {false};
  QPoint _dragOffset {};
};
} // namespace age::visualizer
