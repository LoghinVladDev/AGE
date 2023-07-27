//
// Created by stefan on 23.07.2023.
//

#include "GraphPanel.hpp"

namespace {
using cds::UniquePointer;
using namespace age::visualizer;
} // namespace

GraphPanel::GraphPanel(QWidget* pParent) noexcept : QWidget(pParent) {}

auto GraphPanel::mousePressEvent(QMouseEvent* pEvent) -> void {
  QWidget::mousePressEvent(pEvent);
  auto mousePos = pEvent->position();
  _vertexList.pushBack(cds::makeUnique<Vertex>(mousePos.x(), mousePos.y(), this))->show();
}