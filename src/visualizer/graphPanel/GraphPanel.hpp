//
// Created by stefan on 23.07.2023.
//

#pragma once

#include <CDS/LinkedList>
#include <CDS/memory/UniquePointer>

#include <QMouseEvent>
#include <vertex/Vertex.hpp>
#include <vertexMenu/VertexMenu.hpp>

namespace age::visualizer {
class GraphPanel : public QWidget {
  Q_OBJECT
public:
  explicit GraphPanel(QWidget* pParent = nullptr) noexcept;
  ~GraphPanel() noexcept override = default;
public slots:
  auto menuPopup(QPointF const& point) -> void;

protected:
  auto mousePressEvent(QMouseEvent* pEvent) -> void override;

private:
  cds::LinkedList<cds::UniquePointer<Vertex>> _vertexList;
  cds::UniquePointer<VertexMenu> _vertexMenu;

  cds::UniquePointer<QAction> _deleteAction;
};
} // namespace age::visualizer
