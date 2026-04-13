//
// Created by oleub on 01.02.26.
//

#include "vfs_entry.hpp"
#include "IOStoreReader.hpp"
#include "PAKReader.hpp"
#include "vfs.hpp"
#include "hash/hash.hpp"
namespace SAA {
  uint64_t UnifiedVfsEntry::hashPath(const std::string_view path) {
    return SC::hash_lowercase(path);
  }

  std::span<uint8_t> IOVfsEntry::read(SC::ChaosBumpArena *storage) const { return owningReader->readChunk(storage, chunkId); }

  std::span<uint8_t> PAKVfsEntry::read(SC::ChaosBumpArena *storage) const { return owningReader->readFile(storage,index); }
} 
