//
// Created by stefan on 21.07.2023.
//

#pragma once

#include <QPainter>
#include <QWidget>

namespace age::visualizer {
class Vertex : public QWidget {
  Q_OBJECT
public:
  explicit Vertex(int x, int y, QWidget* pParent) noexcept;

protected:
  auto mousePressEvent(QMouseEvent* pEvent) -> void override;
  auto paintEvent(QPaintEvent* pEvent) -> void override;

private:
  static constexpr int VERTEX_RADIUS = 30;
  static constexpr int RESIZE_OFFSET = 2;
};
} // namespace age::visualizer