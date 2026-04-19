#pragma once
#include <vector>

#include "byte_cursor.hpp"
#include "Reader.hpp"
#include "file_reader.hpp"

#include "game_file.hpp"
#include "pak/f_pak_entry.hpp"
#include "pak/f_pak_info.hpp"

namespace SAA {
  class PAKReader : public Reader {
  public:
    PAKReader(const std::filesystem::path& pakPath);

    PAKReader(const PAKReader &) = delete;
    PAKReader(PAKReader &&) = delete;
    PAKReader &operator=(const PAKReader &) = delete;
    PAKReader &operator=(PAKReader &&) = delete;

    bool load() override;
    bool mountVFS(Vfs *vfs) override;
    GameFile * readFile(SC::ChaosBumpArena *storage, uint32_t index) const;

  private:
    void readPaKInfo();
    std::vector<CompressionMethod> readCompressionMethods(ByteCursor &cursor);
    uint32_t readPakEntry(ByteCursor &cursor, uint32_t offset);

    FileReader m_pakFile;
    FPakInfo m_pakInfo{};
    std::vector<FPakEntry> m_pakEntries;
    std::unique_ptr<SC::ChaosBumpArena> stringStorage;
  };
} 
