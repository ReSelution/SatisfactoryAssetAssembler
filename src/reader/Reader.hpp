#pragma once

#include <cstdint>
#include <filesystem>
#include <span>
#include <utility>
#include "huge_page_resource.hpp"
#include "sc/memory/monotonic_resource.hpp"


namespace SAA {
  class Vfs;

  class Reader {
  public:
    enum class Type : uint8_t {
      IO_STORE = 0,
      PAK,
    };

    Reader(const Reader &)            = delete;
    Reader(Reader &&)                 = delete;
    Reader &operator=(const Reader &) = delete;
    Reader &operator=(Reader &&)      = delete;

    // m_storage steht vor m_Path, path ist bei seiner Initialisierung also noch nicht verschoben.
    Reader(Type type, std::filesystem::path path)
        : initBufferSize(std::filesystem::file_size(path)),
          m_storage(static_cast<size_t>(alocMutipilyer * static_cast<double>(initBufferSize)), HugePageResource::get()),
          m_Path(std::move(path)),
          m_type(type) {}

    virtual ~Reader() = default;

    Type type() { return m_type; }
    virtual bool load();

    [[nodiscard]] const std::filesystem::path &getPath() const { return m_Path; }

    virtual bool mountVFS(Vfs *vfs) = 0;
    // Groesse des ersten Blocks von m_storage: die Dateigroesse mal alocMutipilyer.
    uint64_t initBufferSize;
    double alocMutipilyer = 1.1;
    sc::MonotonicResource m_storage;

  protected:
    std::filesystem::path m_Path;
    std::span<uint8_t> loadFile(const std::filesystem::path &filePath);


  private:
    Type m_type;
  };
} // namespace SAA
