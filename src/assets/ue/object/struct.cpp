//
// Created by oleub on 02.01.26.
//

#include "struct.hpp"

#include <iostream>
#include <ostream>

namespace SAP {
  void Struct::parse() {
    Field::parse();
    m_superStructIndex = m_cursor.read<int32_t>();
  }
} // namespace SAP
