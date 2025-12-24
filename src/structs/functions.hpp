#pragma once

//
// Created by oleub on 10.12.25.
//

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <string>

namespace SAP {


  template<typename T>
  T readStruct(const std::vector<uint8_t> &buffer, size_t &cursor) {
    static_assert(std::is_trivial_v<T>, "T must be a POD struct");

    if (cursor + sizeof(T) > buffer.size())
      throw std::runtime_error("readStruct out of range");

    T value;
    memcpy(&value, buffer.data() + cursor, sizeof(T));
    cursor += sizeof(T);

    return value;
  }

  template<typename T>
  std::vector<T> readArray(const std::vector<uint8_t> &buffer, size_t &cursor, size_t count) {
    static_assert(std::is_trivial_v<T>, "T must be POD");

    size_t total = sizeof(T) * count;

    if (cursor + total > buffer.size())
      throw std::runtime_error("readArray out of range");

    std::vector<T> out(count);
    memcpy(out.data(), buffer.data() + cursor, total);

    cursor += total;
    return out;
  }

  template<typename T>
  T readPrimitive(const std::vector<uint8_t> &buffer, size_t &cursor) {
    static_assert(std::is_arithmetic_v<T> || std::is_enum_v<T>, "T must be primitive or enum");

    if (cursor + sizeof(T) > buffer.size())
      throw std::runtime_error("readPrimitive out of range");

    T value;
    memcpy(&value, buffer.data() + cursor, sizeof(T));

    cursor += sizeof(T);
    return value;
  }

  template<typename T>
  std::vector<T> readCArrayView(const std::vector<uint8_t> &buffer, size_t &cursor) {
    size_t initialPos = cursor;

    auto arrayNum = readPrimitive<uint32_t>(buffer, cursor);
    auto offset = readPrimitive<uint32_t>(buffer, cursor);

    if (arrayNum == 0)
      return {};
    size_t continuePos = cursor;
    cursor = initialPos + offset;
    auto result = readArray<T>(buffer, cursor, arrayNum);

    cursor = continuePos;
    return result;
  }

  template<typename T, typename F>
  std::vector<T> readCArrayView(const std::vector<uint8_t> &buffer, size_t &cursor, F getter) {
    size_t initialPos = cursor;

    auto arrayNum = readPrimitive<uint32_t>(buffer, cursor);
    auto offset = readPrimitive<uint32_t>(buffer, cursor);
    if (arrayNum == 0)
      return {};
    size_t continuePos = cursor;
    cursor = initialPos + offset;
    std::vector<T> result;
    result.reserve(arrayNum);
    for (uint32_t i = 0; i < arrayNum; i++)
      result.push_back(getter());
    cursor = continuePos;
    return result;
  }

  inline std::string utf16ToUtf8(const std::vector<char16_t> &input) {
    std::vector<uint8_t> out(input.size()*2);
    char16_t codepoint = 0;
    for (unsigned char c : input) {
      if (c < 0x80) {
        codepoint = c;
      } else {
        continue;
      }

      out.push_back(static_cast<uint8_t>(codepoint & 0xFF));       // low byte
      out.push_back(static_cast<uint8_t>((codepoint >> 8) & 0xFF)); // high byte
    }
    return {out.begin(), out.end()};
  }



  inline std::string readFString(const std::vector<uint8_t> &buffer, size_t &offset) {
    if (offset + sizeof(int32_t) > buffer.size()) { return ""; }

    int32_t length = 0;
    std::memcpy(&length, buffer.data() + offset, sizeof(int32_t));
    offset += sizeof(int32_t);

    if (length == 0) { return ""; }

    bool isUnicode = length < 0;
    uint32_t absLength = std::abs(length);
    uint32_t byteSize = absLength * (isUnicode ? 2 : 1);

    if (byteSize > buffer.size() - offset) {return "";}

    std::string result;
    if (isUnicode) {
      for (int i = 0; i < absLength - 1; i++) { result += (char) buffer[offset + (static_cast<long>(i) * 2)]; }
    } else {
      if (absLength > 1) { result.assign(reinterpret_cast<const char *>(buffer.data() + offset), absLength - 1); }
    }

    offset += byteSize;
    return result;
  }

  inline char to_lower(char c) {
    // gleich wie C#:
    // return (char)((uint)input + ((((uint)input - 'A' < 26u) ? 1 : 0) << 5));
    const auto uc = static_cast<unsigned>(c);
    return static_cast<char>(uc + (((uc - 'A' < 26u) ? 1u : 0u) << 5));
  }

  inline std::string to_lower(const std::string& in) {
    std::string out;
    out.resize(in.size());

    for (size_t i = 0; i < in.size(); ++i)
      out[i] = to_lower(in[i]);

    return out;
  }

  inline std::vector<uint8_t> utf8_to_utf16le_ascii(const std::string& input)
  {
    std::vector<uint8_t> out;
    out.reserve(input.size() * 2);

    for (unsigned char c : input)
    {
      // ASCII → UTF-16 = 2 bytes: low, high
      out.push_back(c);     // low byte
      out.push_back(0x00);  // high byte (ASCII → 0)
    }

    return out;
  }
} // SAP
