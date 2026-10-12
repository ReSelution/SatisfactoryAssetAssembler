//
// Created by oleub on 31.12.25.
//

#pragma once
#include <cstdint>
#include <span>

#include "byte_cursor.hpp"
#include "f_byte_bulk_data_header.hpp"

namespace SAA {
  class GameFile;

  struct FByteBulkData {
    EBulkDataFlags flags{};
    // The payload in the storage of the file. Empty if there is none, or if it was parsed with
    // EBulkLoad::Deferred and lies in a separate file: readInto() fetches it then.
    ByteCursor bulkCursor;
    // Length of the payload in bytes, also when it is not loaded.
    uint64_t size = 0;
    // Where a payload in a separate file lies. nullptr for an inline payload and if there is no file.
    GameFile *file  = nullptr;
    uint64_t offset = 0;

    // Whether there is a payload that bulkCursor or readInto() can deliver.
    [[nodiscard]] bool hasPayload() const { return bulkCursor.size() != 0 || (file != nullptr && size != 0); }

    // Writes the payload to dst, which has to be size bytes long: decompressed right into dst if it is not loaded
    // yet (GameFile::readInto()), copied from bulkCursor otherwise. Throws if dst has the wrong length, there is no
    // payload or it cannot be read.
    void readInto(std::span<uint8_t> dst) const;
  };

} // namespace SAA
