//
// Created by stefan on 21.07.2023.
//

#include "Vertex.hpp"

namespace {
using namespace age::visualizer;
} // namespace

Vertex::Vertex(QPoint const& point, QWidget* pParent) noexcept : Vertex(point.x(), point.y(), pParent) {}

Vertex::Vertex(int x, int y, QWidget* pParent) noexcept : QWidget(pParent) {
  move(x - WIDGET_SIZE / 2, y - WIDGET_SIZE / 2);
  resize(WIDGET_SIZE, WIDGET_SIZE);
  setPalette(DEFAULT_VERTEX_COLOR);
}

auto Vertex::paintEvent(QPaintEvent*) -> void {
  QPainter painter(this);
  painter.setPen(palette().color(backgroundRole()));
  painter.drawEllipse((WIDGET_SIZE - DRAWN_VERTEX_DIAMETER) / 2, (WIDGET_SIZE - DRAWN_VERTEX_DIAMETER) / 2,
                      DRAWN_VERTEX_DIAMETER, DRAWN_VERTEX_DIAMETER);
}

auto Vertex::mousePressEvent(QMouseEvent* pEvent) -> void {
  if (QPoint vertexCentre {WIDGET_SIZE / 2, WIDGET_SIZE / 2};
      sqrt((vertexCentre.x() - pEvent->position().x()) * (vertexCentre.x() - pEvent->position().x())
           + (vertexCentre.x() - pEvent->position().x()) * (vertexCentre.x() - pEvent->position().x()))
      > DRAWN_VERTEX_DIAMETER / 2.0f) {
    return;
  } else {
    switch (pEvent->button()) {
      case Qt::LeftButton: {
        handleLeftClickEvent(pEvent);
        break;
      }
      case Qt::RightButton: {
        emit rightClickPressed(mapToParent(pEvent->position()));
        break;
      }
      default: {
        QWidget::mousePressEvent(pEvent);
      }
    }
  }
}

auto Vertex::mouseMoveEvent(QMouseEvent* pEvent) -> void {
  if (!(pEvent->buttons() & Qt::LeftButton)) {
    return;
  }
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

auto Vertex::handleLeftClickEvent(QMouseEvent* pEvent) -> void {
  setPalette(SELECTED_VERTEX_COLOR);
  _selected = true;
  _dragOffset = pEvent->position().toPoint();
  raise();
}
