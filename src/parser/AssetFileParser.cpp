//
// Created by oleub on 09.02.26.
//
#include "AssetFileParser.hpp"

#include <stdexcept>
#include <string>

#include "AssetParser.hpp"
#include "game_file.hpp"
#include "version.hpp"
#include "vfs_entry.hpp"
#include "usmap/f_usmap_mapping_data.hpp"

namespace SAA {
  AssetFileParser::~AssetFileParser() { sc::ArenaPool::release(m_storage); }

  void AssetFileParser::reset() {
    // Both live in m_storage.
    m_asset = nullptr;
    m_bulk  = nullptr;
    sc::ArenaPool::release(m_storage);
    m_state.store(ParserState::NONE, std::memory_order_release);
  }

  void AssetFileParser::safeParse() {
    if (m_state.load(std::memory_order_acquire) == ParserState::LOADED) {
      return;
    }
    ParserState expected = ParserState::NONE;
    if (m_state.compare_exchange_strong(expected, ParserState::PARSING, std::memory_order_acq_rel)) {
      const auto publish = [this](ParserState state) {
        {
          std::lock_guard lock(m_mutex);
          m_state.store(state, std::memory_order_release);
        }
        m_cv.notify_all();
      };

      try {
        m_storage = sc::ArenaPool::acquire();
        parse();
      } catch (...) {
        // The threads waiting below have to be released as well.
        publish(ParserState::FAILED);
        throw;
      }
      publish(ParserState::LOADED);
      return;
    }
    {
      std::unique_lock lock(m_mutex);
      m_cv.wait(lock, [this] { return m_state.load(std::memory_order_acquire) >= ParserState::LOADED; });
    }
    if (m_state.load(std::memory_order_acquire) == ParserState::FAILED) [[unlikely]] {
      throw std::runtime_error("Parsing failed earlier: " + std::string(m_path));
    }
  }

  void AssetFileParser::safeReset() {
    if (m_state.load(std::memory_order_acquire) == ParserState::NONE) {
      return;
    }
    ParserState current = m_state.load(std::memory_order_relaxed);
    while (current != ParserState::NONE && current != ParserState::RESETTING) {
      if (m_state.compare_exchange_strong(current, ParserState::RESETTING, std::memory_order_acq_rel)) {
        reset();

        {
          std::lock_guard lock(m_mutex);
          m_state.store(ParserState::NONE, std::memory_order_release);
        }
        m_cv.notify_all();
        return;
      }
    }

    std::unique_lock<std::mutex> lock(m_mutex);
    m_cv.wait(lock, [this] {
      return m_state.load(std::memory_order_acquire) == ParserState::NONE;
    });
  }

  const FUsmapMappingData *AssetFileParser::mappings() const { return m_assetParser->mappings(); }

  EUE5Version AssetFileParser::getUE5Version() const {
    return mappings()->pkgVersion.FileVersionUE5;
  }

  const GlobalData *AssetFileParser::globalData() const { return m_assetParser->globalData(); }

  AssetParser *AssetFileParser::assetParser() const {
    return m_assetParser;
  }

  void AssetFileParser::setReader(Reader *reader) {
    if (m_state.load(std::memory_order_acquire) == ParserState::NONE) {
      m_reader = reader;
    }
  };

  GameFile *AssetFileParser::getBulkFile() {
    if (!m_bulk && m_bulkEntry) {
      m_bulk = m_bulkEntry->read(m_storage.get());
      m_bulk->setOwner(this);
    }
    return m_bulk;
  }

  GameFile *AssetFileParser::getAssetFile() {
    if (!m_asset && m_assetEntry) {
      m_asset = m_assetEntry->read(m_storage.get());
      m_asset->setOwner(this);
    }
    return m_asset;
  }

  ByteCursor AssetFileParser::getAsset(size_t offset, size_t size) {
    return getAssetFile()->subspan(offset, size);
  }
}
