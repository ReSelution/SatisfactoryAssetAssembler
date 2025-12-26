//
// Created by oleub on 24.12.25.
//

#pragma once
#include <atomic>

#include "flags/TextureExportFlags.hpp"


namespace SAP {
  struct ExporterOptions {
    std::atomic<TextureExportFlags> textureExportTypes{
      TextureExportFlags::PNG};
    bool overwriteExisting{false};
  };

  class Exporter {
  public:
    static const ExporterOptions &GetOptions() { return s_options; }

  private:
    static inline ExporterOptions s_options;
  };
}; // SAP
