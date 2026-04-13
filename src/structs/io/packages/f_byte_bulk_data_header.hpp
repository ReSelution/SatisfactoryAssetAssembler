//
// Created by oleub on 31.12.25.
//

#pragma once
#include <cstdint>

#include "f_bulk_data_cooked_index.hpp"
#include "io/packages/f_bulk_data_cooked_index.hpp"

namespace SAA {
  enum class EBulkDataFlags : uint32_t {
    BULKDATA_None = 0,
    BULKDATA_PayloadAtEndOfFile = 1 << 0,
    BULKDATA_SerializeCompressedZLIB = 1 << 1,
    BULKDATA_ForceSingleElementSerialization = 1 << 2,
    BULKDATA_SingleUse = 1 << 3,
    BULKDATA_Unused = 1 << 5,
    BULKDATA_ForceInlinePayload = 1 << 6,
    BULKDATA_SerializeCompressed = BULKDATA_SerializeCompressedZLIB,
    BULKDATA_ForceStreamPayload = 1 << 7,
    BULKDATA_PayloadInSeperateFile = 1 << 8,
    BULKDATA_SerializeCompressedBitWindow = 1 << 9,
    BULKDATA_Force_NOT_InlinePayload = 1 << 10,
    BULKDATA_OptionalPayload = 1 << 11,
    BULKDATA_MemoryMappedPayload = 1 << 12,
    BULKDATA_Size64Bit = 1 << 13,
    BULKDATA_DuplicateNonOptionalPayload = 1 << 14,
    BULKDATA_BadDataVersion = 1 << 15,
    BULKDATA_NoOffsetFixUp = 1 << 16,
    BULKDATA_WorkspaceDomainPayload = 1 << 17,
    BULKDATA_LazyLoadable = 1 << 18,
    // BULKDATA_UsesIoDispatcher                   = 1u << 31,
    BULKDATA_DataIsMemoryMapped = 1 << 30,
    BULKDATA_HasAsyncReadPending = 1 << 29,
    BULKDATA_AlwaysAllowDiscard = 1 << 28,
  };

  // Hilfstyp
  using FlagsType = std::underlying_type_t<EBulkDataFlags>;

  // bitweise Operatoren
  constexpr EBulkDataFlags operator|(EBulkDataFlags lhs, EBulkDataFlags rhs) {
    return static_cast<EBulkDataFlags>(static_cast<FlagsType>(lhs) | static_cast<FlagsType>(rhs));
  }

  constexpr EBulkDataFlags operator&(EBulkDataFlags lhs, EBulkDataFlags rhs) {
    return static_cast<EBulkDataFlags>(static_cast<FlagsType>(lhs) & static_cast<FlagsType>(rhs));
  }

  constexpr EBulkDataFlags operator^(EBulkDataFlags lhs, EBulkDataFlags rhs) {
    return static_cast<EBulkDataFlags>(static_cast<FlagsType>(lhs) ^ static_cast<FlagsType>(rhs));
  }

  constexpr EBulkDataFlags operator~(EBulkDataFlags flags) {
    return static_cast<EBulkDataFlags>(~static_cast<FlagsType>(flags));
  }

  // Zuweisungsoperatoren
  inline EBulkDataFlags &operator|=(EBulkDataFlags &lhs, EBulkDataFlags rhs) {
    lhs = lhs | rhs;
    return lhs;
  }

  inline EBulkDataFlags &operator&=(EBulkDataFlags &lhs, EBulkDataFlags rhs) {
    lhs = lhs & rhs;
    return lhs;
  }

  inline EBulkDataFlags &operator^=(EBulkDataFlags &lhs, EBulkDataFlags rhs) {
    lhs = lhs ^ rhs;
    return lhs;
  }

  // Optional: komfortabler Flag-Test
  constexpr bool HasFlag(EBulkDataFlags value, EBulkDataFlags flag) {
    return (value & flag) != EBulkDataFlags::BULKDATA_None;
  }

#pragma pack(push, 1)
  struct FByteBulkDataHeader {
    EBulkDataFlags flags;
    uint32_t elementCount;
    uint32_t sizeOnDisk;
    uint64_t offsetInFile;
    FBulkDataCookedIndex cookedIndex;
  };
#pragma pack(pop)
} 
