//
// Created by oleub on 12.12.25.
//

#include "io_global_data.hpp"

#include "IOStoreReader.hpp"
#include "logger.hpp"

namespace SAA {
  IOGlobalData::IOGlobalData(std::filesystem::path utocPath, std::filesystem::path ucasPath) {
    storeParser = std::make_unique<IOStoreReader>(utocPath, ucasPath);
    storeParser->initBufferSize += 2908111; // Size of the chunk
    storeParser->alocMutipilyer = 1.25;

  }

  IOGlobalData::~IOGlobalData() = default;

  void IOGlobalData::parse(GlobalData &data) {
    auto start = std::chrono::high_resolution_clock::now();
    storeParser->load();
    auto buffer = storeParser->readChunk(storeParser->m_storage.get(), FIoChunkId{.id = 0, .chunkIndex = 0, .chunkType = 5});
    ByteCursor cursor(buffer);
    data.globalNameMap = cursor.loadNameBatch(storeParser->m_storage.get());

    auto scriptObjectEntries = cursor.readArray<FScriptObjectEntry>();
    data.ScriptObjectEntriesMap.reserve(scriptObjectEntries.size());
    for (auto &scriptObject: scriptObjectEntries) {
      data.ScriptObjectEntriesMap[scriptObject.globalIndex] = scriptObject;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    Logger::logInfo("[GlobalData]", "GlobalNameMap: {} ScriptObjectEntriesMap {} in {}ms", data.globalNameMap.size(),
                    scriptObjectEntries.size(), us.count() / 1000);

  }
} // namespace SAP
