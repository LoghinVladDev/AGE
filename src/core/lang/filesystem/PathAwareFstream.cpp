//
// Created by Vlad-Andrei Loghin on 07.07.23.
//

#include "PathAwareFstream.hpp"
#include <CDS/Array>
#include <filesystem>
#include <platform/PathUtils.hpp>

namespace {
using cds::Array;
using cds::StringView;
using std::string_view;
using std::filesystem::create_directories;
} // namespace

namespace age::meta {
PathAwareDirectoryCreator::PathAwareDirectoryCreator(StringView path) noexcept(false) {
  create_directories(string_view(path.cStr(), path.findLast(directorySeparator)));
}
} // namespace age::meta