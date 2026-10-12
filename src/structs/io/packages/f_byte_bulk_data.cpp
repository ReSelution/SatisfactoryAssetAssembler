#include "f_byte_bulk_data.hpp"

#include <cstring>
#include <stdexcept>

#include "game_file.hpp"

namespace SAA {
  void FByteBulkData::readInto(std::span<uint8_t> dst) const {
    if (dst.size() != size) [[unlikely]] {
      throw std::runtime_error("bulk data target has the wrong size");
    }
    if (bulkCursor.size() != 0) {
      std::memcpy(dst.data(), bulkCursor.buffer().data(), dst.size());
      return;
    }
    if (file == nullptr) [[unlikely]] {
      throw std::runtime_error("bulk data has no payload");
    }
    file->readInto(offset, dst);
  }
} // namespace SAA
