//
// Created by oleub on 01.02.26.
//

#pragma once
#include <cstdint>
#include <string>
#include <utility>

#include "io/f_io_chunk_id.hpp"

namespace SAP {
  class PAKReader;
  class IOStoreReader;

  inline uint64_t fnv1a_lowercase(std::string_view str) {
    uint64_t hash = 0xcbf29ce484222325;
    for (char c: str) {
      char lc;
      if ((c >= 'A' && c <= 'Z')) { lc = c + 32; } else { lc = c; }
      hash ^= static_cast<uint64_t>(lc);
      hash *= 0x100000001b3;
    }
    return hash;
  }

  enum class VFS_EntryType : uint8_t {
    IOVfsEntry,
    UnifiedVfsEntry,
    PAKVfsEntry,
  };

  class UnifiedVfsEntry {
  public:
    virtual ~UnifiedVfsEntry() = default;
    std::string_view path;
    uint64_t hash;
    VFS_EntryType type;

    UnifiedVfsEntry
    (std::string_view pathRef, VFS_EntryType type) : path{pathRef}, type(type) { hash = hashPath(path); };
    static uint64_t hashPath(std::string_view path);

    [[nodiscard]] virtual bool isValid() const { return false; };

    virtual bool operator==(const UnifiedVfsEntry &other) const { return false; };

    bool operator!=(const UnifiedVfsEntry &other) const { return !this->operator==(other); }

    virtual std::vector<uint8_t> read() const { return {}; };
  };

  class IOVfsEntry : public UnifiedVfsEntry {
  public:
    FIoChunkId chunkId;
    IOStoreReader *owningReader;

    IOVfsEntry
    (const std::string_view path, FIoChunkId chunkId, IOStoreReader *owningReader) : UnifiedVfsEntry
      (
       path,
       VFS_EntryType::IOVfsEntry), chunkId{chunkId},
      owningReader(owningReader) {};

    bool operator==(const UnifiedVfsEntry &other) const override {
      if (other.type == VFS_EntryType::IOVfsEntry) {
        auto entry = reinterpret_cast<const IOVfsEntry *>(&other);
        return entry->chunkId == chunkId && entry->owningReader == owningReader;
      }
      return false;
    }

    [[nodiscard]] bool isValid() const override { return owningReader != nullptr; };

    std::vector<uint8_t> read() const  override;
  };

  class PAKVfsEntry : public UnifiedVfsEntry {
  public:
    uint32_t index;
    PAKReader *owningReader;

    PAKVfsEntry(const std::string_view &path, uint32_t index, PAKReader *owningReader) : UnifiedVfsEntry(path, VFS_EntryType::PAKVfsEntry), index(index), owningReader(owningReader) {};

    [[nodiscard]] bool isValid() const override { return owningReader != nullptr; };
    std::vector<uint8_t> read() const override ;
  };
} // SAP
