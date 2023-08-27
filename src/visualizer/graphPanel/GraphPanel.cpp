//
// Created by stefan on 23.07.2023.
//

#include "GraphPanel.hpp"

namespace {
using cds::UniquePointer;
using namespace age::visualizer;
} // namespace

GraphPanel::GraphPanel(QWidget* pParent) noexcept :
    QWidget(pParent), _vertexMenu(cds::makeUnique<VertexMenu>(this)),
    _deleteAction(cds::makeUnique<QAction>(tr("&Delete"), _vertexMenu)) {
  //TODO create AbstractSelection of vertices to be passed to all actions triggered from the context menu
  QObject::connect(_deleteAction, &QAction::triggered, []() {
    // empty on purpose
  });
  _vertexMenu->addAction(_deleteAction);
}

auto GraphPanel::mousePressEvent(QMouseEvent* pEvent) -> void {
  QWidget::mousePressEvent(pEvent);
  auto mousePos = pEvent->position();
  auto newVertex = cds::makeUnique<Vertex>(mousePos.x(), mousePos.y(), this);
  QObject::connect(newVertex, &Vertex::rightClickPressed, this, &GraphPanel::menuPopup);
  _vertexList.pushBack(std::move(newVertex))->show();
}

auto GraphPanel::menuPopup(QPointF const& point) -> void { _vertexMenu->exec(mapToGlobal(point).toPoint()); }
