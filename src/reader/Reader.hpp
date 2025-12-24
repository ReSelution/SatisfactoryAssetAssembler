#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <utility>
#include <vector>

#include "vfs.hpp"

namespace SAP {

  class Reader {
  public:
    struct UnifiedVfsEntry;
    enum class Type : uint8_t {
      IO_STORE = 0,
      PAK,
    };

    Reader(const Reader &) = delete;
    Reader(Reader &&) = delete;
    Reader &operator=(const Reader &) = delete;
    Reader &operator=(Reader &&) = delete;
    Reader(Type type, std::filesystem::path path)
        : m_type(type), m_Path(std::move(path)), m_file(m_Path, std::ios::binary | std::ios::ate) {}
    virtual ~Reader() = default;
    Type type() { return m_type; }
    virtual bool load() = 0; // reine virtuelle Funktion zum Laden
    [[nodiscard]] const std::filesystem::path &getPath() const { return m_Path; }
    virtual bool mountVFS(Vfs &vfs) = 0;
    virtual std::vector<uint8_t> readChunk(const FIoChunkId &chunkId) = 0;


  protected:
    std::filesystem::path m_Path;
    std::ifstream m_file;
    bool readRawBytes(long offset, long size, uint8_t *outBuffer);
    std::streamsize fileSize() {
      m_file.seekg(0, std::ios::end);
      return m_file.tellg();
    }

  private:
    Type m_type;
  };
} // namespace SAP