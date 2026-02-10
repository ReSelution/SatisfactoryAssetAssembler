#include "Reader.hpp"

#include <fstream>

#include "logger.hpp"

namespace SAP {
  std::vector<uint8_t> Reader::loadFile(const std::filesystem::path &filePath) {
    std::ifstream stream(filePath, std::ios::binary);
    stream.seekg(0, std::ios::end);
    std::vector<uint8_t> data(stream.tellg());
    stream.seekg(0, std::ios::beg);
    stream.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));

    return data;
  }
} // namespace SAP
