//
// Created by oleub on 07.01.26.
//

#pragma once
#include <vector>

#include "f_packed_hierachy_node.hpp"
#include "f_page_streaming_state.hpp"
#include "nanite_constants.hpp"

#include "io/packages/f_byte_bulk_data.hpp"

namespace SAP {
  struct FNaniteResources {
    NaniteResourceFlag resourceFlag;
    FByteBulkData streamablePages;
    std::span<const uint8_t> rootPages;
    std::span<const FPageStreamingState> pageStreamingStates;
    std::span<const FPackedHierarchyNode> hierarchyNodes;
    std::span<const uint32_t> rootHierarchyOffset;
    std::span<const uint32_t> pageDependency;
    std::span<const uint16_t> imposterAtlas;
    uint32_t numRootPages;
    int32_t positionPrecision;
    int32_t normalPrecision;
    uint32_t numInputTriangles;
    uint32_t numInputVertices;
    uint16_t numInputMeshes;
    uint16_t numTexCoords;
    uint32_t numClusters;
  };
} // namespace SAP
