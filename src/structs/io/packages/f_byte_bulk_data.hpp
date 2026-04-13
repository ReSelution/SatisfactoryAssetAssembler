//
// Created by oleub on 31.12.25.
//

#pragma once
#include "byte_cursor.hpp"
#include "f_byte_bulk_data_header.hpp"

namespace SAA {
  struct FByteBulkData {
    EBulkDataFlags flags;
    ByteCursor bulkCursor;
  };

} 
