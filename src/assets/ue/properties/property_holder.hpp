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
  enum class EPropertyTagType : uint64_t;

  enum class ReadType : uint8_t {
    NORMAL,
    MAP,
    ARRAY,
  };


  class PropertyHolder {
    using denseMap = ankerl::unordered_dense::pmr::map<uint64_t, FPropertyTag, sc::IdentityHash>;

  public:
    virtual ~PropertyHolder() = default;

    PropertyHolder(std::string_view className = "PropertyHolder");
    ;

    PropertyHolder(denseMap *props);
    ;
    denseMap *m_properties = nullptr;

    void parseProperties(ByteCursor &cursor);


    static PropertyHolder parseFallbackStruct(ByteCursor &cursor);

    template<typename T>
    std::optional<T> get(uint64_t hash) const;

    template<typename T, typename Func>
    std::optional<T> get(uint64_t hash, Func &&processor) const;


    template<typename T>
    std::span<T> getArray(uint64_t hash) const;

    template<typename T, typename Func>
    std::span<T> getArray(uint64_t hash, Func &&processor) const;


  protected:
    AssetFileParser *m_owner = nullptr;
    std::string m_className;
  };
} // namespace SAA
