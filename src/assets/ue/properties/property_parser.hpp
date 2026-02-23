//
// Created by oleub on 23.02.26.
//

#pragma once
#include <cstdint>

#include "byte_cursor.hpp"

namespace SAP {
  class PropertyHolder;

  template<uint32_t Version>
  class PropertyParser {
  public:
    static void parse(PropertyHolder *holder, ByteCursor &cursor) {
      throw std::runtime_error("PropertyParser for Version " +
                               std::to_string(Version) +
                               " not implemented yet!");
    }
  };

  template<>
    void PropertyParser<1009>::parse(PropertyHolder *holder, ByteCursor &cursor);
}
