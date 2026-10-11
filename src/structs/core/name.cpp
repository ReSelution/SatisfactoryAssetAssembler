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

  std::string_view FName::text() const {
    if (number == 0) [[likely]] {
      return plainTextView;
    }

    // TODO: Allokiert bei number != 0 bei jedem Aufruf neu, seit der generatedView-Cache raus ist. Der Speicher
    // wird erst mit der Resource freigegeben, in einer heissen Schleife waechst sie also mit jedem Aufruf.

    if (!slab || plainTextView.empty()) [[unlikely]] {
      return "ERR_INVALID_DATA";
    }

    const size_t suffixSize = 2 + digits_needed(number);
    const size_t prefixSize = plainTextView.size();
    const size_t needed     = prefixSize + suffixSize;

    char *mem = static_cast<char *>(slab->allocate(needed, alignof(char)));
    // TODO: Toter Code. sc::MonotonicResource::allocate wirft bei Fehlschlag std::bad_alloc und gibt nie nullptr
    // zurueck.
    if (!mem) [[unlikely]]
      return plainTextView;

    std::memcpy(mem, plainTextView.data(), prefixSize);
    mem[prefixSize] = '_';

    char *numStart = mem + prefixSize + 1;
    char *numLimit = mem + needed;

    // Direct offset formatting
    auto [ptr, ec] = std::to_chars(numStart, numLimit, number - 1);

    return std::string_view(mem, static_cast<size_t>(ptr - mem));
  }
} // namespace SAA
