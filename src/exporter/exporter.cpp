//
// Created by oleub on 24.12.25.
//

#include "exporter.hpp"

namespace SAP {
  bool Exporter::checkPath(std::filesystem::path path) {
    std::filesystem::path dir(path.parent_path());
    if (!dir.empty() && !std::filesystem::exists(dir)) {
      std::filesystem::create_directories(dir);
    }
    return std::filesystem::exists(path);
  }
} // namespace SAP
