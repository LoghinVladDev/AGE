//
// Created by Vlad-Andrei Loghin on 18.06.23.
//

#include "VisualizerWindow.hpp"
#include <QVBoxLayout>
//#include <QGraphicsScene>

namespace age::visualizer {
VisualizerWindow::VisualizerWindow(QWidget* pParent) noexcept :
    QWidget(pParent), _primaryLayout(cds::makeUnique<QVBoxLayout>(this)) {
  resize(500, 500);
  auto graphPanel = new GraphPanel();
  this->_primaryLayout->addWidget(graphPanel);
}
} // namespace age::visualizer
