//
// Created by oleub on 01.02.26.
//

#pragma once

#include "io/f_io_chunk_id.hpp"

namespace SAA {
  class GameFile;
  class PAKReader;
  class IOStoreReader;


  enum class VFS_EntryType : uint8_t {
    IOVfsEntry,
    UnifiedVfsEntry,
    PAKVfsEntry,
  };

  class UnifiedVfsEntry {
  public:
    virtual ~UnifiedVfsEntry() = default;

    std::string_view pathStr;
    bool isPkg = false;
    std::string_view packagePath;
    uint64_t hash;


    UnifiedVfsEntry(std::string_view pathRef) : pathStr{pathRef}, packagePath(cleanPath(pathStr)),
                                                hash(hashPath(packagePath)) {
    };

    static std::string_view cleanPathStatic(std::string_view path);

    std::string_view cleanPath(std::string_view path);

    static uint64_t hashPath(std::string_view path);

    [[nodiscard]] virtual VFS_EntryType getEntryType() const { return VFS_EntryType::UnifiedVfsEntry; }

    [[nodiscard]] virtual bool isValid() const { return false; };

    virtual bool operator==(const UnifiedVfsEntry &other) const { return false; };

    bool operator!=(const UnifiedVfsEntry &other) const { return !this->operator==(other); }

    virtual GameFile *read(SC::ChaosBumpArena *storage) const = 0;
  };

  class IOVfsEntry : public UnifiedVfsEntry {
  public:
    FIoChunkId chunkId;
    IOStoreReader *owningReader;

    IOVfsEntry(const std::string_view path, FIoChunkId chunkId, IOStoreReader *owningReader) : UnifiedVfsEntry(path),
                                                                                               chunkId{chunkId},
                                                                                               owningReader(
                                                                                                 owningReader) {
    };

    [[nodiscard]] VFS_EntryType getEntryType() const override { return VFS_EntryType::IOVfsEntry; }


    [[nodiscard]] bool isValid() const override { return owningReader != nullptr; };

    GameFile *read(SC::ChaosBumpArena *storage) const override;

    bool operator==(const UnifiedVfsEntry &other) const override {
      if (other.getEntryType() == VFS_EntryType::IOVfsEntry) {
        auto entry = static_cast<const IOVfsEntry *>(&other);
        return entry->chunkId == chunkId && entry->owningReader == owningReader;
      }
      return false;
    }
  };

  class PAKVfsEntry : public UnifiedVfsEntry {
  public:
    static constexpr VFS_EntryType type = VFS_EntryType::PAKVfsEntry;
    uint32_t index;
    PAKReader *owningReader;

    PAKVfsEntry(const std::string_view &path, uint32_t index, PAKReader *owningReader) : UnifiedVfsEntry(path),
                                                                                         index(index),
                                                                                         owningReader(owningReader) {
    };

    VFS_EntryType getEntryType() const override { return VFS_EntryType::PAKVfsEntry; }
    [[nodiscard]] bool isValid() const override { return owningReader != nullptr; };

    GameFile *read(SC::ChaosBumpArena *storage) const override;
  };
}
