//
// Created by stefan on 23.07.2023.
//

#include "GraphPanel.hpp"

namespace {
using cds::UniquePointer;
using namespace age::visualizer;
} // namespace

GraphPanel::GraphPanel(QWidget* pParent) noexcept : QWidget(pParent), _vertexMenu(cds::makeUnique<VertexMenu>(this)) {}

auto GraphPanel::mousePressEvent(QMouseEvent* pEvent) -> void {
  QWidget::mousePressEvent(pEvent);
  auto mousePos = pEvent->position();
  auto newVertex = cds::makeUnique<Vertex>(mousePos.x(), mousePos.y(), this);
  QObject::connect(newVertex, &Vertex::rightClickPressed, _vertexMenu, &VertexMenu::popup);
  _vertexList.pushBack(std::move(newVertex))->show();
}
