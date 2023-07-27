//
// Created by stefan on 24.07.2023.
//

#include <gtest/gtest.h>
#include <visualizer/vertex/Vertex.hpp>

namespace {
using age::visualizer::Vertex;
}

TEST(VertexTest, construction) { Vertex v(0, 0, nullptr); }

struct TestVertex : Vertex {
  TestVertex() : Vertex(0, 0, nullptr) {}
  void paintEvent(QPaintEvent* pEvent) override { this->Vertex::paintEvent(pEvent); };
  void mousePressEvent(QMouseEvent* pEvent) override { this->Vertex::mousePressEvent(pEvent); };
};

TEST(VertexTest, mousePressEvent) {
  TestVertex t;
  t.mousePressEvent({});
}

TEST(VertexTest, paintEvent) {
  TestVertex t;
  t.paintEvent({});
}
