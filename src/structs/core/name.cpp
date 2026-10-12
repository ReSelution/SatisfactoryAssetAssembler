#include "name.hpp"
#include "sc/memory/monotonic_resource.hpp"

#include <charconv>
#include <cstring>

//
// Created by oleub on 19.03.26.
//
namespace SAA {

  [[nodiscard]] static constexpr size_t digits_needed(uint32_t n) {
    if (n < 10)
      return 1;
    if (n < 100)
      return 2;
    if (n < 1000)
      return 3;
    if (n < 10000)
      return 4;
    if (n < 100000)
      return 5;
    if (n < 1000000)
      return 6;
    if (n < 10000000)
      return 7;
    if (n < 100000000)
      return 8;
    if (n < 1000000000)
      return 9;
    return 10;
  }

  std::string_view FMappedName::fromNameMap(std::span<std::string_view> map, sc::MonotonicResource *storage) const {
    auto index = NameIndex();

    if (index >= map.size()) {
      return "None";
    }

    std::string_view base = map[index];
    if (extraIndex == 0) {
      return base;
    }
    constexpr size_t maxSuffix = 1 + 10;
    const uint32_t baseSize    = base.size();
    size_t totalSize           = baseSize + maxSuffix;
    auto buf                   = storage->allocateSpan<char>(totalSize);
    std::memcpy(buf.data(), base.data(), baseSize);

    char *ptr = buf.data();
    ptr += base.size();

    *ptr = '_';
    ptr++;

    auto [next, ec] = std::to_chars(ptr, buf.data() + buf.size(), extraIndex - 1);
    if (ec != std::errc{}) {
      return base;
    }
    size_t finalLen = next - buf.data();
    return {buf.data(), finalLen};
  }

  std::string_view FName::numbered(std::string_view base, uint32_t number, sc::MonotonicResource *storage) {
    const size_t prefixSize = base.size();
    const size_t needed     = prefixSize + 1 + digits_needed(number - 1);

    char *mem = static_cast<char *>(storage->allocate(needed, alignof(char)));
    std::memcpy(mem, base.data(), prefixSize);
    mem[prefixSize] = '_';

    // Cannot fail, needed has room for every digit.
    auto [ptr, ec] = std::to_chars(mem + prefixSize + 1, mem + needed, number - 1);
    (void) ec;
    return {mem, static_cast<size_t>(ptr - mem)};
  }
} // namespace SAA
