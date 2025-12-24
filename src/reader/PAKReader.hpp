#pragma once
#include <cstdint>
#include <filesystem>
#include <vector>

#include "Reader.hpp"
#include "vfs.hpp"

namespace SAP {
  class PAKReader : public Reader {
  public:
    PAKReader(std::filesystem::path pakPath);
    ~PAKReader();

    PAKReader(const PAKReader &) = delete;
    PAKReader(PAKReader &&) = delete;
    PAKReader &operator=(const PAKReader &) = delete;
    PAKReader &operator=(PAKReader &&) = delete;

    bool load() override;
    bool mountVFS(Vfs &vfs) override;
    std::vector<uint8_t> readChunk(const FIoChunkId &chunkId) override { return {}; }

  private:
  };
} // namespace SAP
