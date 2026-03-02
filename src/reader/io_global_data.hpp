//
// Created by oleub on 12.12.25.
//

#pragma once
#include <deque>
#include <filesystem>
#include <unordered_map>
#include <vector>

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
    std::unordered_map<FPackageObjectIndex, FScriptObjectEntry> ScriptObjectEntriesMap;
  };

  class IOGlobalData {
  public:
    IOGlobalData(std::filesystem::path utocPath, std::filesystem::path ucasPath);
    ~IOGlobalData();

    void parse(GlobalData &data);

  private:
    std::unique_ptr<IOStoreReader> storeParser;
  };
} // namespace SAP
