//
// Created by stefan on 21.07.2023.
//

#include "Vertex.hpp"

namespace age::visualizer {
Vertex::Vertex(int x, int y, QWidget* pParent) noexcept : QWidget(pParent) {
  move(x - VERTEX_DIAMETER / 2, y - VERTEX_DIAMETER / 2);
  resize(VERTEX_DIAMETER, VERTEX_DIAMETER);
  setPalette(DEFAULT_VERTEX_COLOR);
}

auto Vertex::paintEvent(QPaintEvent*) -> void {
  QPainter painter(this);
  painter.setPen(palette().color(backgroundRole()));
  painter.drawEllipse(0, 0, VERTEX_DIAMETER - PEN_WIDTH_OFFSET, VERTEX_DIAMETER - PEN_WIDTH_OFFSET);
}

auto Vertex::mousePressEvent(QMouseEvent* pEvent) -> void {
  if (QPoint vertexCentre {VERTEX_DIAMETER / 2, VERTEX_DIAMETER / 2};
      sqrt(((vertexCentre.x() - pEvent->pos().x()) * (vertexCentre.x() - pEvent->pos().x())
            + (vertexCentre.x() - pEvent->pos().x()) * (vertexCentre.x() - pEvent->pos().x())))
      > VERTEX_DIAMETER / 2.0f) {
    QWidget::mousePressEvent(pEvent);
  }

  switch (pEvent->button()) {
    case Qt::LeftButton: {
      handleLeftClickEvent(pEvent);
      break;
    }
    case Qt::RightButton: {
      emit rightClickPressed(this, mapToParent(pEvent->pos()));
      break;
    }
    default: {
      QWidget::mousePressEvent(pEvent);
    }
  }
}

auto Vertex::mouseMoveEvent(QMouseEvent* pEvent) -> void {
  if (!(pEvent->buttons() & Qt::LeftButton)) {
    return;
  }
  if (_selected) {
    move(mapToParent(pEvent->pos() - _dragOffset));
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
  _dragOffset = pEvent->pos();
  raise();
}
} // namespace age::visualizer
