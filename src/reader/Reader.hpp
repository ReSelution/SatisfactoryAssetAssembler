#pragma once

#include <cstdint>
#include <filesystem>
#include <utility>
#include <vector>


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

    Reader(Type type, std::filesystem::path path)
        : m_type(type), m_Path(std::move(path)){}

    virtual ~Reader() = default;

    Type type() { return m_type; }
    virtual bool load() = 0; // reine virtuelle Funktion zum Laden

    [[nodiscard]] const std::filesystem::path &getPath() const { return m_Path; }

    virtual bool mountVFS(Vfs *vfs) = 0;


  protected:
    std::filesystem::path m_Path;
    std::vector<uint8_t> loadFile(const std::filesystem::path &filePath);


  private:
    Type m_type;
  };
} // namespace SAP