//
// Created by oleub on 09.02.26.
//
#include "AssetFileParser.hpp"

#include "AssetParser.hpp"
#include "vfs_entry.hpp"

namespace SAA {
  AssetFileParser::~AssetFileParser() { HeapPool::release(m_storage);}
  const FUsmapMappingData *AssetFileParser::mappings() const { return m_assetParser->mappings(); }
  const GlobalData *AssetFileParser::globalData() const { return m_assetParser->globalData(); }

  AssetParser * AssetFileParser::assetParser() const {
    return m_assetParser;
  }

  void AssetFileParser::setReader(Reader *reader) {
    if (m_state.load(std::memory_order_acquire) == ParserState::NONE) {
      m_reader = reader;
    }
  };

  ByteCursor AssetFileParser::getAsset() {
    if (m_asset.empty() && m_assetEntry) {
      m_asset = m_assetEntry->read(m_storage.get());
    }
    return ByteCursor(m_asset, this);
  }

  ByteCursor AssetFileParser::getBulk() {
    if (m_bulk.empty() && m_bulkEntry) {
      m_bulk = m_bulkEntry->read(m_storage.get());
    }
    return ByteCursor(m_bulk);
  }
} // namespace SAP
