//
// Created by Vlad-Andrei Loghin on 18.06.23.
//

#include "VisualizerWindow.hpp"
#include <QVBoxLayout>

namespace age::visualizer {
VisualizerWindow::VisualizerWindow(QWidget* pParent) noexcept :
    QWidget(pParent), _primaryLayout(cds::makeUnique<QVBoxLayout>(this)) {
  resize(200, 300);
}
void VisualizerWindow::mousePressEvent(QMouseEvent* event) { std::cout<<"Clicked\n"; QWidget::mousePressEvent(event);   }
} // namespace age::visualizer
