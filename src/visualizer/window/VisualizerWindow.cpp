//
// Created by Vlad-Andrei Loghin on 18.06.23.
//

#include "VisualizerWindow.hpp"
#include <QVBoxLayout>

namespace age::visualizer {
VisualizerWindow::VisualizerWindow(QWidget* pParent) noexcept :
    QWidget(pParent), _primaryLayout(cds::makeUnique<QVBoxLayout>(this)),
    _graphPanel(cds::makeUnique<GraphPanel>(this)) {
  resize(500, 500);
  _primaryLayout->addWidget(_graphPanel);
}
} // namespace age::visualizer
