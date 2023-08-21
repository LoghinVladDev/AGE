//
// Created by stefan on 8/7/23.
//

#include "VertexMenu.hpp"

namespace {
using namespace age::visualizer;
} // namespace

VertexMenu::VertexMenu(QWidget* pParent) noexcept : QWidget(pParent) {
  resize(55, 30);
  setPalette(Qt::black);
  hide();
}

void VertexMenu::popup(Vertex* pVertex, QPoint const& qPoint) {
  move(qPoint);
  show();
  raise();
}

void VertexMenu::paintEvent(QPaintEvent* event) {
  QPainter painter(this);
  painter.setPen(palette().color(backgroundRole()));
  painter.fillRect(QRect(0, 0, 50, 25), Qt::gray);
  painter.drawRect(QRect(0, 0, 50, 25));
}
