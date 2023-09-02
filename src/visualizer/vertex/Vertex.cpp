//
// Created by stefan on 21.07.2023.
//

#include "Vertex.hpp"

namespace {
using namespace age::visualizer;
} // namespace

Vertex::Vertex(int x, int y, QWidget* pParent) noexcept : QWidget(pParent) {
  move(x - WIDGET_SIZE / 2, y - WIDGET_SIZE / 2);
  resize(WIDGET_SIZE, WIDGET_SIZE);
  setPalette(DEFAULT_VERTEX_COLOR);
}

Vertex::Vertex(QPoint const& point, QWidget* pParent) noexcept : Vertex(point.x(), point.y(), pParent) {}

auto Vertex::paintEvent(QPaintEvent*) -> void {
  QPainter painter(this);
  painter.setPen(palette().color(backgroundRole()));
  painter.drawEllipse((WIDGET_SIZE - DRAWN_VERTEX_DIAMETER) / 2, (WIDGET_SIZE - DRAWN_VERTEX_DIAMETER) / 2,
                      DRAWN_VERTEX_DIAMETER, DRAWN_VERTEX_DIAMETER);
}

auto Vertex::mousePressEvent(QMouseEvent* pEvent) -> void {
  if (distanceToCenter(pEvent->position().toPoint()) > DRAWN_VERTEX_DIAMETER / 2.0f) {
    return;
  }
  switch (pEvent->button()) {
    case Qt::LeftButton: handleLeftClickEvent(pEvent); break;
    case Qt::RightButton: emit rightClickPressed(mapToParent(pEvent->position())); break;
    default: {
      QWidget::mousePressEvent(pEvent);
    }
  }
}

auto Vertex::mouseMoveEvent(QMouseEvent* pEvent) -> void {
  if (_selected) {
    move(mapToParent(pEvent->position().toPoint() - _dragOffset));
  }
}

auto Vertex::mouseReleaseEvent(QMouseEvent* pEvent) -> void {
  if (_selected) {
    setPalette(DEFAULT_VERTEX_COLOR);
    _selected = false;
  }
}

auto Vertex::handleLeftClickEvent(QMouseEvent const* pEvent) -> void {
  setPalette(SELECTED_VERTEX_COLOR);
  _selected = true;
  _dragOffset = pEvent->position().toPoint();
  raise();
}

constexpr auto Vertex::distanceToCenter(QPoint const& point) -> double {
  auto distToCenter = WIDGET_SIZE / 2;
  auto distPointToCenterX = distToCenter - point.x();
  auto distPointToCenterY = distToCenter - point.y();
  return std::sqrt(distPointToCenterX * distPointToCenterX + distPointToCenterY * distPointToCenterY);
}
