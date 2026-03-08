//
// Created by oleub on 09.02.26.
//

#pragma once
#include <cstdint>
#include <optional>
#include <unordered_map>

#include "byte_cursor.hpp"
#include "core/hash.hpp"

namespace SAA {
  struct FPropertyValue;
  struct FPropertyTag;
  struct FPropertyTagData;
  enum class EPropertyTagType : uint32_t;

  enum class ReadType : uint8_t {
    NORMAL,
    MAP,
    ARRAY,
  };



  class PropertyHolder {
  public:
    virtual ~PropertyHolder() = default;

    PropertyHolder(std::string_view className = "PropertyHolder");;

    PropertyHolder(std::pmr::memory_resource *resource);;
    std::pmr::unordered_map<uint32_t, FPropertyTag, IdentityHash> m_properties;

    void parseProperties(ByteCursor &cursor);

    static void releaseResources();

    static PropertyHolder parseFallbackStruct(ByteCursor &cursor);

    template<typename T>
    std::optional<T> get(uint32_t hash) const;

    template<typename T, typename Func>
    std::optional<T> get(uint32_t hash, Func &&processor) const;


    template<typename T>
    std::span<T> getArray(uint32_t hash) const;

    template<typename T, typename Func>
    std::span<T> getArray(uint32_t hash, Func &&processor) const;

  protected:
    AssetFileParser *m_owner = nullptr;
    std::string m_className;


  };
} // namespace SAP
