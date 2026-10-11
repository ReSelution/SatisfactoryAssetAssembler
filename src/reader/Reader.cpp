#include "Reader.hpp"

#include <fstream>
#include <span>

namespace SAA {
  bool Reader::load() {
    return true;
  }

  std::span<uint8_t> Reader::loadFile(const std::filesystem::path &filePath) {
    std::ifstream stream(filePath, std::ios::binary);
    stream.seekg(0, std::ios::end);
    auto data = m_storage.allocateSpan(static_cast<size_t>(stream.tellg()));
    stream.seekg(0, std::ios::beg);
    stream.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
    stream.close();
    return data;
  }
} 
