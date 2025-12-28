//
// Created by oleub on 12.12.25.
//

#include "io_global_data.hpp"

#include "IOStoreReader.hpp"
#include "logger.hpp"

namespace SAP {
  IOGlobalData::IOGlobalData(std::filesystem::path utocPath, std::filesystem::path ucasPath) {
    storeParser = std::make_unique<IOStoreReader>(utocPath, ucasPath);
  }

  IOGlobalData::~IOGlobalData() = default;

  void IOGlobalData::parse(GlobalData &data) {
    auto result = storeParser->load();
    Logger::logInfo("[GlobalData]", "storeParser: {}", result);
    size_t cursor = 0;

    auto buffer = storeParser->readChunk(FIoChunkId{.id=0,.chunkIndex= 0, .chunkType = 5});
    data.globalNameMap = loadNameBatch(buffer,cursor);

    auto numScriptObjects = readPrimitive<uint32_t>(buffer,cursor);
    auto scriptObjectEntries = readArray<FScriptObjectEntry>(buffer,cursor,numScriptObjects);
    data.ScriptObjectEntriesMap.reserve(numScriptObjects);
    for (auto &scriptObject: scriptObjectEntries) {
      data.ScriptObjectEntriesMap[scriptObject.globalIndex] = scriptObject;
    }
    Logger::logInfo("[GlobalData]", "globalNameMap: {} ScriptObjectEntriesMap {}", data.globalNameMap.size(), numScriptObjects );

  }
} // SAP