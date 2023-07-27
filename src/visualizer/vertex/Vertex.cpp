//
// Created by stefan on 21.07.2023.
//

#include "Vertex.hpp"

namespace age::visualizer {
Vertex::Vertex(int x, int y, QWidget* pParent) noexcept : QWidget(pParent) {
  move(x - VERTEX_RADIUS / 2, y - VERTEX_RADIUS / 2);
  resize(VERTEX_RADIUS + RESIZE_OFFSET, VERTEX_RADIUS + RESIZE_OFFSET);
  setPalette(Qt::blue);
}

auto Vertex::paintEvent(QPaintEvent*) -> void {
  QPainter painter(this);
  painter.setPen(palette().color(backgroundRole()));
  painter.drawEllipse(0, 0, VERTEX_RADIUS, VERTEX_RADIUS);
}

auto Vertex::mousePressEvent(QMouseEvent*) -> void { setPalette(Qt::red); }
} // namespace age::visualizer
