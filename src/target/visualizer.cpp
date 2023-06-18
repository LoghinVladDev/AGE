//
// Created by Vlad-Andrei Loghin on 17.06.23.
//

#include <QApplication>
#include <QWidget>

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  QWidget w;
  w.resize(100, 100);
  w.show();
  return QApplication::exec();
}
