//
// Created by stefan on 24.07.2023.
//

#include <gtest/gtest.h>
#include <visualizer/vertex/Vertex.hpp>

namespace {
using age::visualizer::Vertex;
}

TEST(VertexTest, construction) { Vertex v(0, 0, nullptr); }
TEST(VertexTest, qPointConstruction) { Vertex v(QPoint {}, nullptr); }

struct TestVertex : Vertex {
  TestVertex() : Vertex(0, 0, nullptr) {}
  void paintEvent(QPaintEvent* pEvent) override { this->Vertex::paintEvent(pEvent); }
  void mousePressEvent(QMouseEvent* pEvent) override { this->Vertex::mousePressEvent(pEvent); }
  void mouseMoveEvent(QMouseEvent* pEvent) override { this->Vertex::mouseMoveEvent(pEvent); }
  void mouseReleaseEvent(QMouseEvent* pEvent) override { this->Vertex::mouseReleaseEvent(pEvent); }
};

TEST(VertexTest, mousePressEvent) {
  TestVertex t;
  auto clickOutsideEvent = QMouseEvent(Qt::LeftButton, QPointF {});
  t.mousePressEvent(&clickOutsideEvent);

  auto leftClickInsideEvent = QMouseEvent(Qt::LeftButton, QPointF {20.0, 20.0});
  t.mousePressEvent(&leftClickInsideEvent);

  auto rightClickInsideEvent = QMouseEvent(Qt::RightButton, QPointF {20.0, 20.0});
  t.mousePressEvent(&rightClickInsideEvent);

  auto middleClickInsideEvent = QMouseEvent(Qt::MiddleButton, QPointF {20.0, 20.0});
  t.mousePressEvent(&middleClickInsideEvent);
}

TEST(VertexTest, mouseMoveEvent) {
  TestVertex t;
  QMouseEvent mouseMoveEvent;
  t.mouseMoveEvent(&mouseMoveEvent);

  auto leftClickInsideEvent = QMouseEvent(Qt::LeftButton, QPointF {20.0, 20.0});
  t.mousePressEvent(&leftClickInsideEvent);
  t.mouseMoveEvent(&mouseMoveEvent);
}

TEST(VertexTest, mouseReleaseEvent) {
  TestVertex t;

  QMouseEvent mouseReleaseEvent;
  t.mouseReleaseEvent(&mouseReleaseEvent);

  auto leftClickInsideEvent = QMouseEvent(Qt::LeftButton, QPointF {20.0, 20.0});
  t.mousePressEvent(&leftClickInsideEvent);
  t.mouseReleaseEvent(&mouseReleaseEvent);
}

TEST(VertexTest, paintEvent) {
  TestVertex t;
  t.paintEvent({});
}
