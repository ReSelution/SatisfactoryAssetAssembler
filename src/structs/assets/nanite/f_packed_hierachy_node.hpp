//
// Created by oleub on 07.01.26.
//

#pragma once
#include <array>

#include "f_hierarchy_node_slice.hpp"
#include "nanite_constants.hpp"

namespace SAA {
#pragma pack(push, 1)
  struct FPackedHierarchyNode {
    std::array<FHierarchyNodeSlice, NANITE_MAX_BVH_NODE_FANOUT> slices;
  };
#pragma pack(pop)
} // namespace SAP
