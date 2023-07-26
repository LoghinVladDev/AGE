//
// Created by stefan on 23.07.2023.
//

#include "GraphPanel.hpp"

namespace age::visualizer {
GraphPanel::GraphPanel(QWidget* pParent) noexcept : QWidget(pParent) {}

void GraphPanel::mousePressEvent(QMouseEvent* pEvent) {
  auto mousePos = pEvent->position();
  cds::UniquePointer<Vertex> vertex = cds::makeUnique<Vertex>((int) mousePos.x(), (int) mousePos.y(), this);
  vertex->show();
  _vertexList.pushBack(std::move(vertex));
}

} // namespace age::visualizer
