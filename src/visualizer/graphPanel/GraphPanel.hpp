//
// Created by stefan on 23.07.2023.
//

#ifndef AGE_GRAPH_PANEL_HPP
#define AGE_GRAPH_PANEL_HPP

#include <QMouseEvent>
#include <vertex/Vertex.hpp>

namespace age::visualizer {
class GraphPanel : public QWidget {
  Q_OBJECT
public:
  explicit GraphPanel(QWidget* pParent = nullptr) noexcept;

protected:
  void mousePressEvent(QMouseEvent* pEvent) override;
};
} // namespace age::visualizer

#endif //AGE_GRAPH_PANEL_HPP
