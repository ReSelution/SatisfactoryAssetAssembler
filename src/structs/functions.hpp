#pragma once

//
// Created by oleub on 10.12.25.
//

#include <cstdint>
#include <cstring>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

#include "byte_cursor.hpp"

namespace SAA {

  inline char to_lower(char c) {
    // gleich wie C#:
    // return (char)((uint)input + ((((uint)input - 'A' < 26u) ? 1 : 0) << 5));
    const auto uc = static_cast<unsigned>(c);
    return static_cast<char>(uc + (((uc - 'A' < 26u) ? 1u : 0u) << 5));
  }

  inline std::string to_lower(const std::string_view in) {
    std::string out;
    out.resize(in.size());

    for (size_t i = 0; i < in.size(); ++i)
      out[i] = to_lower(in[i]);

    return out;
  }

  inline std::vector<uint8_t> utf8_to_utf16le_ascii(const std::string &input) {
    std::vector<uint8_t> out;
    out.reserve(input.size() * 2);

    for (unsigned char c: input) {
      // ASCII → UTF-16 = 2 bytes: low, high
      out.push_back(c);    // low byte
      out.push_back(0x00); // high byte (ASCII → 0)
    }

    return out;
  }
} 
