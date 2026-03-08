#include "Reader.hpp"

#include <fstream>

#include "cache/chaos_slab.hpp"

namespace SAA {
  bool Reader::load() {
    m_storage = std::make_unique<SC::ChaosSlab>(alocMutipilyer * initBufferSize);

    return true;
  }

  std::span<uint8_t> Reader::loadFile(const std::filesystem::path &filePath) {
    std::ifstream stream(filePath, std::ios::binary);
    stream.seekg(0, std::ios::end);
    auto data = m_storage->allocateSpan(stream.tellg());
    stream.seekg(0, std::ios::beg);
    stream.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
    stream.close();
    return data;
  }
} // namespace SAP
