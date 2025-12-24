#include "Reader.hpp"

#include <fstream>

namespace SAP {
  bool Reader::readRawBytes(long offset, long size, uint8_t *outBuffer) {
    m_file.seekg(offset, std::ios::beg);
    if (m_file.fail()) { return false; }

    m_file.read(reinterpret_cast<char *>(outBuffer), size);
    return !m_file.fail() || m_file.gcount() == size;
  }
} // namespace SAP
