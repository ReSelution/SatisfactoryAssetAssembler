//
// Created by oleub on 12.12.25.
//

#include "io_global_data.hpp"

#include "IOStoreReader.hpp"
#include "logger/chaos_logger.hpp"

LOG_ALIAS(GlobalDataLog, "SAA", "GlobalData")

namespace SAA {
  IOGlobalData::IOGlobalData(std::filesystem::path utocPath, std::filesystem::path ucasPath) {
    storeParser = std::make_unique<IOStoreReader>(utocPath, ucasPath);
    storeParser->initBufferSize += 2908111; // Size of the chunk
    storeParser->alocMutipilyer = 1.25;
  }

  IOGlobalData::~IOGlobalData() = default;

  void IOGlobalData::parse(GlobalData &data) {
    size_t globalNameMapSize{0};
    size_t scriptObjectEntriesSize{0};
    auto t = GlobalDataLog::time("GlobalNameMap: {1} ScriptObjectEntriesMap {2} in {0}", globalNameMapSize,
                                 scriptObjectEntriesSize);
    storeParser->load();
    auto buffer = storeParser->readChunk(storeParser->m_storage.get(),
                                         FIoChunkId{.id = 0, .chunkIndex = 0, .chunkType = 5});
    ByteCursor cursor(buffer);
    data.globalNameMap = cursor.loadNameBatch(storeParser->m_storage.get());

    auto scriptObjectEntries = cursor.readArray<FScriptObjectEntry>();
    data.ScriptObjectEntriesMap.reserve(scriptObjectEntries.size());
    for (auto &scriptObject: scriptObjectEntries) {
      data.ScriptObjectEntriesMap[scriptObject.globalIndex] = scriptObject;
    }
    globalNameMapSize = data.globalNameMap.size();
    scriptObjectEntriesSize = data.ScriptObjectEntriesMap.size();
  }
} // namespace SAP
