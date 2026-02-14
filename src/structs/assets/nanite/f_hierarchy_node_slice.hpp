//
// Created by oleub on 07.01.26.
//

#pragma once
#include <cstdint>
#include <type_traits>

#include "core/math/f_vector.hpp"

namespace SAP {
#pragma pack(push, 1)

  struct FHierarchyNodeSlice {
    // Bounds
    FVector4_f LODBounds;      // 4 floats
    FVector_f BoxBoundsCenter; // 3 floats

    uint16_t MinLODError;       // Half (raw)
    uint16_t MaxParentLODError; // Half (raw)

    FVector_f BoxBoundsExtent; // 3 floats

    // Hierarchy / children
    uint32_t ChildStartReference; // 0xFFFFFFFF = unloaded
    uint32_t Misc2;               // packed bitfield (NumChildren, NumPages, StartPageIndex, flags)
  };

#pragma pack(pop)

  static_assert(std::is_trivial_v<FHierarchyNodeSlice>);
  static_assert(std::is_standard_layout_v<FHierarchyNodeSlice>);
} // namespace SAP
