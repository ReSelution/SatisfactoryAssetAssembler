//
// Created by oleub on 12.12.25.
//

#include "io_global_data.hpp"

#include "IOStoreReader.hpp"
#include "sc/logger/logger.hpp"

using GlobalDataLog = sc::Logger<"SAA", "GlobalData">;

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
    auto file =
        storeParser->readChunk(&storeParser->m_storage, FIoChunkId{.id = 0, .chunkIndex = 0, .chunkType = 5});
    data.globalNameMap = file->loadNameBatch();

    auto scriptObjectEntries = file->readArray<FScriptObjectEntry>();
    data.scriptObjectEntriesMap.reserve(scriptObjectEntries.size());
    for (auto &scriptObject: scriptObjectEntries) {
      data.scriptObjectEntriesMap[scriptObject.globalIndex] = scriptObject;
    }
    globalNameMapSize       = data.globalNameMap.size();
    scriptObjectEntriesSize = data.scriptObjectEntriesMap.size();
  }
} // namespace SAA
