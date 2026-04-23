//
// Created by oleub on 07.01.26.
//

#pragma once

#include "f_packed_hierachy_node.hpp"
#include "f_page_streaming_state.hpp"
#include "nanite_constants.hpp"
#include "glm/mat3x4.hpp"
#include "core/f_box_sphere_bounds.hpp"

#include "io/packages/f_byte_bulk_data.hpp"

namespace SAA {
  struct FNaniteResources {
    NaniteResourceFlag resourceFlag;
    FByteBulkData *streamablePages;
    std::span<const uint8_t> rootPages;
    std::span<const FPageStreamingState> pageStreamingStates;
    std::span<const FPackedHierarchyNode> hierarchyNodes;
    std::span<const uint32_t> rootHierarchyOffset;
    std::span<const uint32_t> pageDependency;
    std::span<const uint16_t> imposterAtlas;
    std::span<const glm::mat3x4> assemblyTransforms;
    FBoxSphereBoundsF meshBounds;
    uint32_t numRootPages;
    int32_t positionPrecision;
    int32_t normalPrecision;
    uint32_t numInputTriangles;
    uint32_t numInputVertices;
    uint16_t numInputMeshes;
    uint16_t numTexCoords;
    uint32_t numClusters;
  };
} 
