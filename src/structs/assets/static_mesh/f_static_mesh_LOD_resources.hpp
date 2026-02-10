//
// Created by oleub on 07.01.26.
//

#pragma once
#include <vector>

#include "f_card_represantation_data.hpp"
#include "f_static_mesh_section.hpp"
#include "f_static_mesh_vertex_buffer.hpp"
#include "f_static_raw_index_buffer.hpp"

#include "assets/meshes/f_color_vertex_buffer.hpp"
#include "assets/meshes/f_postion_vertex_Buffer.hpp"

namespace SAP {
  enum class EFStaticMeshLODResourcesStripFlag : uint8_t {
    CDSF_AdjacencyData       = 1,
    CDSF_MinLodData          = 2,
    CDSF_ReversedIndexBuffer = 4,
    CDSF_RayTracingResources = 8,
  };

  struct FStaticMeshLODResources {
    std::span<const FStaticMeshSection> sections;
    float maxDeviation;
    FPositionVertexBuffer positionVertexBuffer;
    FStaticMeshVertexBuffer vertexBuffer;
    FColorVertexBuffer colorVertexBuffer;
    FRawStaticIndexBuffer indexBuffer;
    FRawStaticIndexBuffer reversedIndexBuffer;
    FRawStaticIndexBuffer depthOnlyIndexBuffer;
    FRawStaticIndexBuffer reversedDepthOnlyIndexBuffer;
    FRawStaticIndexBuffer wireframeIndexBuffer;
    FRawStaticIndexBuffer adjacentIndexBuffer;
    FCardRepresentationData cardRepresentationData;
  };
}
