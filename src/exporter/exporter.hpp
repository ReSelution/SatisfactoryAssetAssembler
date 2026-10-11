//
// Created by oleub on 24.12.25.
//

#pragma once
#include <atomic>
#include <filesystem>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <utility>

#include "flags/MeshExportFlags.hpp"
#include "flags/TextureExportFlags.hpp"

#include "sc/ecs/entity.hpp"
#include "sc/ecs/registry.hpp"
#include "sc/logger/logger.hpp"

using exLog = sc::Logger<"SAA", "Exporter">;

namespace SAA {
  class Object;

  struct ExporterOptions {
    std::atomic<TextureExportFlags> textureExportTypes{TextureExportFlags::PNG};
    std::atomic<MeshExportFlags> meshExportTypes{MeshExportFlags::GLTF2};
    std::string outputPath = "./Exports";
    mutable std::shared_mutex pathMutex;

    std::filesystem::path getPath() const {
      std::shared_lock lock(pathMutex);
      return outputPath;
    }

    void setPath(std::string newPath) {
      std::unique_lock lock(pathMutex);
      outputPath = std::move(newPath);
    }
  };

  class Exporter {
  public:
    static const ExporterOptions &options() { return s_options; }

    static void setECS(sc::ecs::Registry *reg);
    static sc::ecs::Entity getEntity(Object *obj);

  protected:
    static bool checkPath(std::filesystem::path path);
    static inline sc::ecs::Registry *reg = nullptr;

  private:
    static inline ExporterOptions s_options;
  };
} // namespace SAA
