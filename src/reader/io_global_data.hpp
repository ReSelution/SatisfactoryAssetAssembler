//
// Created by oleub on 12.12.25.
//

#pragma once
#include <ankerl/unordered_dense.h>
#include <filesystem>

#include "core/name.hpp"
#include "core/package.hpp"

namespace SAA {
  class IOStoreReader;
  struct FScriptObjectEntry {
    FMappedName ObjectName;
    FPackageObjectIndex globalIndex;
    FPackageObjectIndex outerIndex;
    FPackageObjectIndex CDOClassIndex;
  };

  struct GlobalData {
    std::span<std::string_view> globalNameMap;
    using denseMap = ankerl::unordered_dense::map<FPackageObjectIndex, FScriptObjectEntry>;
    denseMap scriptObjectEntriesMap;
  };

  class IOGlobalData {
  public:
    IOGlobalData(std::filesystem::path utocPath, std::filesystem::path ucasPath);
    ~IOGlobalData();

    void parse(GlobalData &data);

  private:
    std::unique_ptr<IOStoreReader> storeParser;
  };
} // namespace SAA
