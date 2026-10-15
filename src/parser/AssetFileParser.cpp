//
// Created by oleub on 09.02.26.
//
#include "AssetFileParser.hpp"

#include <stdexcept>
#include <string>

#include "sc/memory/arena_pool.hpp"

#include "AssetParser.hpp"
#include "game_file.hpp"
#include "usmap/f_usmap_mapping_data.hpp"
#include "version.hpp"
#include "vfs_entry.hpp"

namespace SAA {
  AssetFileParser::~AssetFileParser() {
    if (m_storage) {
      m_assetParser->m_arenaPool.release(std::move(m_storage));
    }
  }

  uint64_t AssetFileParser::hashText(const FName &name) { return sc::hash(name.text()); }

  void AssetFileParser::setNameMap(std::span<std::string_view> nameMap) {
    m_nameMap    = nameMap;
    m_nameHashes = m_storage->allocateSpan<uint64_t>(nameMap.size());
    for (size_t i = 0; i < nameMap.size(); ++i) {
      m_nameHashes[i] = sc::hash(nameMap[i]);
    }
  }

  void AssetFileParser::reset() {
    // All of them live in m_storage.
    m_asset = nullptr;
    m_bulk = nullptr;
    m_nameHashes = {};
    if (m_storage) {
      m_assetParser->m_arenaPool.release(std::move(m_storage));
    }
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
        m_storage = m_assetParser->m_arenaPool.acquire();
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
    m_cv.wait(lock, [this] { return m_state.load(std::memory_order_acquire) == ParserState::NONE; });
  }

  const FUsmapMappingData *AssetFileParser::mappings() const { return m_assetParser->mappings(); }

  EUE5Version AssetFileParser::getUE5Version() const { return mappings()->pkgVersion.FileVersionUE5; }

  const GlobalData *AssetFileParser::globalData() const { return m_assetParser->globalData(); }

  AssetParser *AssetFileParser::assetParser() const { return m_assetParser; }

  void AssetFileParser::setReader(Reader *reader) {
    if (m_state.load(std::memory_order_acquire) == ParserState::NONE) {
      m_reader = reader;
    }
  };

  GameFile *AssetFileParser::getBulkFile() {
    GameFile *file = m_bulk.load(std::memory_order_acquire);
    if (file == nullptr && m_bulkEntry != nullptr) {
      const std::scoped_lock lock(m_fileLock);
      file = m_bulk.load(std::memory_order_relaxed);
      if (file == nullptr) {
        file = m_bulkEntry->read(m_storage.get());
        file->setOwner(this);
        m_bulk.store(file, std::memory_order_release);
      }
    }
    return file;
  }

  GameFile *AssetFileParser::getAssetFile() {
    GameFile *file = m_asset.load(std::memory_order_acquire);
    if (file == nullptr && m_assetEntry != nullptr) {
      const std::scoped_lock lock(m_fileLock);
      file = m_asset.load(std::memory_order_relaxed);
      if (file == nullptr) {
        file = m_assetEntry->read(m_storage.get());
        file->setOwner(this);
        m_asset.store(file, std::memory_order_release);
      }
    }
    return file;
  }

  ByteCursor AssetFileParser::getAsset(size_t offset, size_t size) { return getAssetFile()->subspan(offset, size); }
} // namespace SAA
