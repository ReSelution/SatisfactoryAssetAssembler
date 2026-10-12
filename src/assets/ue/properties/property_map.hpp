#pragma once
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

// Part of f_property_tag.hpp, which includes this file once FPropertyTag and FPropertyValue are defined. Include
// that one.

namespace sc {
  class MonotonicResource;
}

namespace SAA {
  // What is kept of a property once it is parsed.
  struct FProperty {
    std::string_view Name;
    std::string_view TypeName;
    FPropertyValue Tag;
  };

  // The properties of one object or struct, found by PropertyHolder::propertyKey(). Written once by a
  // PropertyMapBuilder and not changed after that.
  //
  // A plain array that is searched from the front, no hash table: the test parse of 1017 builds 3.07 M of these,
  // 2.65 M for structs, and 91 % of those hold 1 to 4 properties. The keys lie apart from the values, so a search
  // only walks 8 bytes per property.
  class PropertyMap {
  public:
    // The first property with that key, nullptr if there is none.
    [[nodiscard]] const FProperty *find(uint64_t key) const noexcept {
      for (uint32_t i = 0; i < m_size; ++i) {
        if (m_keys[i] == key) {
          return m_values + i;
        }
      }
      return nullptr;
    }

    [[nodiscard]] size_t size() const noexcept { return m_size; }
    [[nodiscard]] std::span<const uint64_t> keys() const noexcept { return {m_keys, m_size}; }
    [[nodiscard]] std::span<const FProperty> values() const noexcept { return {m_values, m_size}; }

  private:
    friend class PropertyMapBuilder;

    const uint64_t *m_keys    = nullptr;
    const FProperty *m_values = nullptr;
    uint32_t m_size           = 0;
  };

  // Collects the properties of one holder while they are parsed and writes them into the storage in one piece, at
  // exactly the size they need. Until then they wait in a buffer of the thread, which the builders of nested
  // structs share like a stack: a builder has to be finished or destroyed before the one around it is used again.
  class PropertyMapBuilder {
  public:
    PropertyMapBuilder();
    PropertyMapBuilder(const PropertyMapBuilder &)            = delete;
    PropertyMapBuilder &operator=(const PropertyMapBuilder &) = delete;
    // Drops what was not built, also when parsing threw.
    ~PropertyMapBuilder();

    // A key that is already there stays findable under its first property only.
    void add(uint64_t key, FPropertyTag &&tag);

    [[nodiscard]] PropertyMap *build(sc::MonotonicResource *storage);

  private:
    size_t m_start;
  };
} // namespace SAA
