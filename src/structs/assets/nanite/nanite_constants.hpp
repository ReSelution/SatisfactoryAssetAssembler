//
// Created by oleub on 07.01.26.
//

#pragma once
#include <cstdint>

namespace SAP {
  // -----------------------------------------------------------------------------
  // Magic
  // -----------------------------------------------------------------------------
  constexpr uint16_t NANITE_FIXUP_MAGIC = 0x464E;

  // -----------------------------------------------------------------------------
  // BVH
  // -----------------------------------------------------------------------------
  constexpr int NANITE_MAX_BVH_NODE_FANOUT_BITS = 2;
  constexpr int NANITE_MAX_BVH_NODE_FANOUT = 1 << NANITE_MAX_BVH_NODE_FANOUT_BITS;

  // -----------------------------------------------------------------------------
  // Clusters / Pages
  // -----------------------------------------------------------------------------
  constexpr int NANITE_MAX_CLUSTERS_PER_GROUP_BITS = 9;
  constexpr int NANITE_MAX_RESOURCE_PAGES_BITS = 20;

  // -----------------------------------------------------------------------------
  // Vertex attributes
  // -----------------------------------------------------------------------------
  constexpr int NANITE_MAX_UVS = 4;

  // Normal / tangent quantization
  constexpr int NANITE_MAX_NORMAL_QUANTIZATION_BITS_500 = 9;
  constexpr int NANITE_MAX_NORMAL_QUANTIZATION_BITS_502 = 15;
  constexpr int NANITE_MAX_TANGENT_QUANTIZATION_BITS = 12;

  // Texcoord quantization
  constexpr int NANITE_MAX_TEXCOORD_QUANTIZATION_BITS_500 = 15;
  constexpr int NANITE_MAX_TEXCOORD_QUANTIZATION_BITS_504 = 20;

  // Vertex color
  constexpr int NANITE_MAX_COLOR_QUANTIZATION_BITS = 8;

  // -----------------------------------------------------------------------------
  // Page / cluster sizing
  // -----------------------------------------------------------------------------
  constexpr int NANITE_CLUSTER_MIN_EXPECTED_GPU_SIZE_BITS = 9;

  // Root page
  constexpr int NANITE_ROOT_PAGE_GPU_SIZE_BITS = 15;
  constexpr int NANITE_ROOT_PAGE_GPU_SIZE = 1 << NANITE_ROOT_PAGE_GPU_SIZE_BITS;
  constexpr int NANITE_ROOT_PAGE_MAX_CLUSTERS_BITS =
      NANITE_ROOT_PAGE_GPU_SIZE_BITS - NANITE_CLUSTER_MIN_EXPECTED_GPU_SIZE_BITS;
  constexpr int NANITE_ROOT_PAGE_MAX_CLUSTERS = 1 << NANITE_ROOT_PAGE_MAX_CLUSTERS_BITS;

  // Streaming page
  constexpr int NANITE_STREAMING_PAGE_GPU_SIZE_BITS = 17;
  constexpr int NANITE_STREAMING_PAGE_GPU_SIZE = 1 << NANITE_STREAMING_PAGE_GPU_SIZE_BITS;
  constexpr int NANITE_STREAMING_PAGE_MAX_CLUSTERS_BITS =
      NANITE_STREAMING_PAGE_GPU_SIZE_BITS - NANITE_CLUSTER_MIN_EXPECTED_GPU_SIZE_BITS;
  constexpr int NANITE_STREAMING_PAGE_MAX_CLUSTERS = 1 << NANITE_STREAMING_PAGE_MAX_CLUSTERS_BITS;

  // Clusters per page (version dependent)
  constexpr int NANITE_MAX_CLUSTERS_PER_PAGE_BITS_500 = 8;
  constexpr int NANITE_MAX_CLUSTERS_PER_PAGE_BITS_504 =
      (NANITE_STREAMING_PAGE_MAX_CLUSTERS_BITS > NANITE_ROOT_PAGE_MAX_CLUSTERS_BITS)
          ? NANITE_STREAMING_PAGE_MAX_CLUSTERS_BITS
          : NANITE_ROOT_PAGE_MAX_CLUSTERS_BITS;

  // -----------------------------------------------------------------------------
  // Cluster indices
  // -----------------------------------------------------------------------------
  constexpr int NANITE_MAX_CLUSTER_INDICES_BITS = 8;
  constexpr int NANITE_MAX_CLUSTER_INDICES = 1 << NANITE_MAX_CLUSTER_INDICES_BITS;
  constexpr int NANITE_MAX_CLUSTER_INDICES_MASK = NANITE_MAX_CLUSTER_INDICES - 1;

  // -----------------------------------------------------------------------------
  // Hierarchy / groups
  // -----------------------------------------------------------------------------
  constexpr int NANITE_MAX_HIERACHY_CHILDREN_BITS = 6;
  constexpr int NANITE_MAX_GROUP_PARTS_BITS = 3;

  constexpr int NANITE_MAX_HIERACHY_CHILDREN = 1 << NANITE_MAX_HIERACHY_CHILDREN_BITS;
  constexpr int NANITE_MAX_GROUP_PARTS_MASK = (1 << NANITE_MAX_GROUP_PARTS_BITS) - 1;

  // -----------------------------------------------------------------------------
  // UV float encoding
  // -----------------------------------------------------------------------------
  constexpr int NANITE_UV_FLOAT_NUM_EXPONENT_BITS = 5;
  constexpr int NANITE_UV_FLOAT_MAX_MANTISSA_BITS = 14;
  constexpr int NANITE_UV_FLOAT_NUM_MANTISSA_BITS = 14;
  constexpr int NANITE_UV_FLOAT_MAX_BITS = 1 + NANITE_UV_FLOAT_NUM_EXPONENT_BITS + NANITE_UV_FLOAT_MAX_MANTISSA_BITS;

  // -----------------------------------------------------------------------------
  // Position precision
  // -----------------------------------------------------------------------------
  constexpr int NANITE_MIN_POSITION_PRECISION_500 = -8;
  constexpr int NANITE_MAX_POSITION_PRECISION_500 = 23;

  constexpr int NANITE_MIN_POSITION_PRECISION_504 = -20;
  constexpr int NANITE_MAX_POSITION_PRECISION_504 = 43;

  // -----------------------------------------------------------------------------
  // Vertex color mode
  // -----------------------------------------------------------------------------
  constexpr int NANITE_VERTEX_COLOR_MODE_VARIABLE = 1;

  enum class NaniteResourceFlag : uint32_t {
    NONE = 0x0,
    HAS_VERTEX_COLOR = 0x1,
    HAS_IMPOSTER = 0x2,
    STREAMING_DATA_IN_DDC = 0x4,
    FORCE_ENABLED = 0x8,
  };

  enum class NanitePageFlag : uint8_t {
    NONE = 0x0,
    RELATIVE_ENCODING = 0x1,
  };
} // namespace SAP
