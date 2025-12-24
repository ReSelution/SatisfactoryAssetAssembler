
#include "PAKReader.hpp"

#include "vfs.hpp"
#include "Reader.hpp"

namespace SAP {
  PAKReader::PAKReader(std::filesystem::path path) : Reader(Reader::Type::PAK, std::move(path)) {}

  bool PAKReader::load() { return true; }

  bool PAKReader::mountVFS(Vfs &vfs) { return true; }

  PAKReader::~PAKReader() {}
} // namespace SAP
