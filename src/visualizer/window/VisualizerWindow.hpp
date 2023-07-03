//
// Created by Vlad-Andrei Loghin on 18.06.23.
//

#ifndef AGE_VISUALIZER_WINDOW_HPP
#define AGE_VISUALIZER_WINDOW_HPP

#include <CDS/memory/UniquePointer>

#include <QLayout>
#include <QWidget>

namespace age::visualizer {
class VisualizerWindow : public QWidget {
  Q_OBJECT
public:
  explicit VisualizerWindow(QWidget* pParent = nullptr) noexcept;
  ~VisualizerWindow() noexcept override = default;
  void mousePressEvent(QMouseEvent *event) override;

private:
  cds::UniquePointer<QLayout> _primaryLayout {nullptr};
};
} // namespace age::visualizer

#endif // AGE_VISUALIZER_WINDOW_HPP
