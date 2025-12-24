//
// Created by oleub on 20.12.25.
//

#pragma once
#include "f_property_tag_data.hpp"
#include "core/name.hpp"

namespace SAP {

  enum class EPropertyTagSerializeType : uint8_t {
    Unknown,
    Skipped,
    Property,
    BinaryOrNative
  };

  enum class EPropertyTagFlags : uint8_t {
    None                       = 0x00,
    HasArrayIndex              = 0x01,
    HasPropertyGuid            = 0x02,
    HasPropertyExtensions      = 0x04,
    HasBinaryOrNativeSerialize = 0x08,
    BoolTrue                   = 0x10,
    SkippedSerialize           = 0x20
  };

#pragma pack(push, 1)
  struct FPropertyTag {
    FName Name;
    EPropertyTagType PropertyType;

    uint32_t Size = 0;
    uint32_t ArrayIndex = 0;
    std::optional<uint32_t> ArraySize;

    std::optional<FGuid> PropertyGuid;
    EPropertyTagFlags Flags = EPropertyTagFlags::None;

    std::unique_ptr<FPropertyTagData> TagData;
    std::unique_ptr<FPropertyValue> Tag;

    [[nodiscard]] bool isIndexed() const { return Flags >= EPropertyTagFlags::HasArrayIndex || ArrayIndex > 0; }

    [[nodiscard]] EPropertyTagSerializeType serializeType() const {
      if (Flags >= EPropertyTagFlags::SkippedSerialize)
        return EPropertyTagSerializeType::Skipped;
      if (Flags >= EPropertyTagFlags::HasBinaryOrNativeSerialize)
        return EPropertyTagSerializeType::BinaryOrNative;
      return EPropertyTagSerializeType::Property;
    }
  };

#pragma pack(pop)
}
