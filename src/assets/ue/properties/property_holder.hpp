//
// Created by oleub on 09.02.26.
//

#pragma once
#include <cstdint>
#include <optional>

#include <ankerl/unordered_dense.h>
#include "byte_cursor.hpp"
#include "sc/hash/hash.hpp"

namespace SAA {
  struct FPropertyValue;
  struct FPropertyTag;
  struct FPropertyTagData;
  struct FProperty;
  class PropertyMap;
  enum class EPropertyTagType : uint64_t;

  enum class ReadType : uint8_t {
    NORMAL,
    MAP,
    ARRAY,
  };


  class PropertyHolder {
  public:
    virtual ~PropertyHolder() = default;

    PropertyHolder() = default;

    PropertyHolder(const PropertyMap *props);
    // Empty until the properties are parsed, and for a struct that is known to have none.
    const PropertyMap *m_properties = nullptr;

    void parseProperties(ByteCursor &cursor);


    static PropertyHolder parseFallbackStruct(ByteCursor &cursor);

    // Key of a property in m_properties. Element 0 of a static array sits under the plain hash of its name like
    // every other property, the elements 1..n get a key of their own. Not collected into one array: an asset only
    // stores the elements that differ from the default, so an index can be missing, element 0 included.
    [[nodiscard]] static constexpr uint64_t propertyKey(uint64_t nameHash, uint32_t arrayIndex) {
      return nameHash ^ (arrayIndex * 0x9E3779B97F4A7C15ull);
    }

    template<typename T>
    std::optional<T> get(uint64_t hash) const;

    // One element of a static array, see propertyKey().
    template<typename T>
    std::optional<T> getIndexed(uint64_t hash, uint32_t arrayIndex) const {
      return get<T>(propertyKey(hash, arrayIndex));
    }

    template<typename T, typename Func>
    std::optional<T> get(uint64_t hash, Func &&processor) const;


    template<typename T>
    std::span<T> getArray(uint64_t hash) const;

    template<typename T, typename Func>
    std::span<T> getArray(uint64_t hash, Func &&processor) const;


  protected:
    [[nodiscard]] const FProperty *findProperty(uint64_t hash) const;

#ifdef SAA_TRACK_PROPERTIES
    // Name the PropertyTracker files the properties under.
    [[nodiscard]] virtual std::string_view trackedClassName() const { return "PropertyHolder"; }
#endif

    // Nothing in here may own memory: as a property value a holder is never destroyed, see the checks behind
    // FPropertyValue in f_property_tag.hpp.
    AssetFileParser *m_owner = nullptr;
  };
} // namespace SAA
