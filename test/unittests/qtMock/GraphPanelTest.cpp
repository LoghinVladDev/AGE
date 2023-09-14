//
// Created by stefan on 24.07.2023.
//

#include <gtest/gtest.h>
#include <visualizer/graphPanel/GraphPanel.hpp>

namespace {
using age::visualizer::GraphPanel;
}

TEST(GraphPanelTest, construction) { GraphPanel g; }

struct TestGraphPanel : GraphPanel {
  void mousePressEvent(QMouseEvent* pEvent) override { this->GraphPanel::mousePressEvent(pEvent); }
};
TEST(GraphPanelTest, mousePressEvent) {
  TestGraphPanel t;
  QMouseEvent e;
  t.mousePressEvent(&e);
}
