//
// Created by Vlad-Andrei Loghin on 18.06.23.
//

#include <gtest/gtest.h>
#include <visualizer/window/VisualizerWindow.hpp>

namespace {
using age::visualizer::VisualizerWindow;
}

TEST(VisualizerWindowTest, construction) { VisualizerWindow w(nullptr); }
