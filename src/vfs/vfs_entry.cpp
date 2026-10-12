//
// Created by oleub on 01.02.26.
//

#include "vfs_entry.hpp"
#include "IOStoreReader.hpp"
#include "PAKReader.hpp"
#include "sc/hash/hash.hpp"

namespace SAA {
  std::string_view UnifiedVfsEntry::cleanPathStatic(std::string_view path) {
    if (path.size() < 7) return path; // Schneller Check für zu kurze Namen
    auto pos = path.find_last_of('.');
    if (pos != std::string_view::npos) [[likely]]{
      std::string_view ext = path.substr(pos);
      if (ext == ".uasset" || ext == ".umap") {
        return path.substr(0, pos);
      }
    }
    return path;
  }

  uint64_t UnifiedVfsEntry::hashPath(const std::string_view path) {
    return sc::hash_lowercase(path);
  }

  GameFile *IOVfsEntry::read(sc::MonotonicResource *storage) const { return owningReader->readChunk(storage, chunkId); }

  GameFile *PAKVfsEntry::read(sc::MonotonicResource *storage) const {
    return owningReader->readFile(storage,index); }
}