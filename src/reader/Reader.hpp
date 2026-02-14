#pragma once

#include <cstdint>
#include <filesystem>
#include <utility>
#include <vector>

#include "heap_storage.hpp"

namespace SAP {
  class Vfs;

  class Reader {
  public:
    enum class Type : uint8_t {
      IO_STORE = 0,
      PAK,
    };

    Reader(const Reader &) = delete;
    Reader(Reader &&) = delete;
    Reader &operator=(const Reader &) = delete;
    Reader &operator=(Reader &&) = delete;

    Reader(Type type, std::filesystem::path path) : m_type(type), m_Path(std::move(path)) {
      initBufferSize = std::filesystem::file_size(m_Path);
    }

    virtual ~Reader() = default;

    Type type() { return m_type; }
    virtual bool load();

    [[nodiscard]] const std::filesystem::path &getPath() const { return m_Path; }

    virtual bool mountVFS(Vfs *vfs) = 0;
    uint64_t initBufferSize;
    double alocMutipilyer = 1.1;
    std::unique_ptr<HeapStorage> m_storage{};

  protected:
    std::filesystem::path m_Path;
    std::span<uint8_t> loadFile(const std::filesystem::path &filePath);



  private:
    Type m_type;
  };
} // namespace SAP
