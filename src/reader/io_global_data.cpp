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
    auto start = std::chrono::high_resolution_clock::now();
    storeParser->load();

    data.buffer = storeParser->readChunk(FIoChunkId{.id=0,.chunkIndex= 0, .chunkType = 5});
    ByteCursor cursor( data.buffer);
    data.globalNameMap = cursor.loadNameBatch(data.utf16Storage);


    auto scriptObjectEntries = cursor.readArray<FScriptObjectEntry>();
    data.ScriptObjectEntriesMap.reserve(scriptObjectEntries.size());
    for (auto &scriptObject: scriptObjectEntries) {
      data.ScriptObjectEntriesMap[scriptObject.globalIndex] = scriptObject;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    Logger::logInfo("[GlobalData]", "GlobalNameMap: {} ScriptObjectEntriesMap {} in {}ms", data.globalNameMap.size(), scriptObjectEntries.size(), us.count()/1000 );
  }
} // SAP