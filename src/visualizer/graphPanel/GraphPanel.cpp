//
// Created by stefan on 23.07.2023.
//

#include "GraphPanel.hpp"

namespace age::visualizer {
GraphPanel::GraphPanel(QWidget* pParent) noexcept : QWidget(pParent) {}

void GraphPanel::mousePressEvent(QMouseEvent* pEvent) {
  GraphPanel::mousePressEvent(pEvent);
  auto mousePos = pEvent->position();
_vertexList.pushBack(cds::makeUnique<Vertex>(mousePos.x(), mousePos.y(), this))->show();
}

} // namespace age::visualizer
