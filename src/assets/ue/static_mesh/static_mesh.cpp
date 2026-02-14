//
// Created by oleub on 06.01.26.
//

#include "static_mesh.hpp"

#include "uasset_parser.hpp"

#include "assets/static_mesh/distance_field_atlas.hpp"

#include "meshes/mesh_exporter.hpp"

#include "ue/material/material_interface.hpp"

#include "usmap/f_usmap_mapping_data.hpp"

namespace SAP {
  void StaticMesh::parse() {
    Object::parse();
    auto stripDataFlags = m_cursor.read<FStripDataFlags>();
    m_cooked = m_cursor.readBool();
    auto index = m_cursor.read<int32_t>();
    m_bodySetup = m_owner->resolveIndex(index);
    index = m_cursor.read<int32_t>();
    m_NavCollision = m_owner->resolveIndex(index);
    m_lightingGuid = m_cursor.read<FGuid>();

    // Sockets
    auto socketsCount = m_cursor.read<uint32_t>();
    auto indexs = m_cursor.readArray<int32_t>(socketsCount);
    m_sockets.reserve(socketsCount);
    for (auto &i: indexs) {
      m_sockets.emplace_back(m_owner->resolveIndex(i));
    }
    if (m_cooked) {
      parseRenderData();
    }
    parseStaticMaterials();
  }

  void StaticMesh::exportAsset() { MeshExporter::exportAsset(this); }

  void StaticMesh::parseDistanceFieldVolumeData() {
    auto stripDataFlags = m_cursor.read<FStripDataFlags>();
    bool stripped = stripDataFlags.isAudioVisualDataStripped() || stripDataFlags.isClassDataStripped(0x01);
    if (!stripped) {
      for (uint32_t i = 0; i < m_renderData.LODResources.size(); ++i) {
        if (m_cursor.readBool()) {
          FDistanceFieldVolumeData5::parse(m_cursor, m_owner);
        }
      }
    }
  }

  void StaticMesh::parseRenderData() {
    auto lenghts = m_cursor.read<uint32_t>();
    m_renderData.LODResources.resize(lenghts);
    for (auto i = 0; i < lenghts; ++i) {
      parseLODResource(m_renderData.LODResources[i]);
    }
    m_cursor += 1; // numInlinedLODs

    parseNaniteResources(m_renderData.naniteResources);
    parseInlineDataRepresentation();
    parseDistanceFieldVolumeData();

    m_renderData.bounds = m_cursor.read<FBoxSphereBounds>();
    m_renderData.bLODsShareStaticLighting = m_cursor.readBool();
    constexpr uint32_t MAX_STATIC_LODS_UE4 = 8;
    m_renderData.screenSize.reserve(MAX_STATIC_LODS_UE4);
    for (uint32_t i = 0; i < MAX_STATIC_LODS_UE4; ++i) {
      m_cursor += 4; // skip bFloatCooked
      m_renderData.screenSize.emplace_back(m_cursor.read<float>());
    }
  }

  void StaticMesh::parseLODResource(FStaticMeshLODResources &lodResources) {
    auto stripDataFlags = m_cursor.read<FStripDataFlags>();
    lodResources.sections = m_cursor.readArray<FStaticMeshSection>();
    SAP_ASSERT_UE5_MAX(m_owner, UE5_5, "FBoxSphereBounds not Implented ");
    lodResources.maxDeviation = m_cursor.read<float>();
    bool isLODCookedOut = m_cursor.readBool();
    bool inlined = m_cursor.readBool();
    if (!stripDataFlags.isAudioVisualDataStripped() && !isLODCookedOut) [[likely]] {
      SAP_ASSERT_UE5_MAX(m_owner, UE5_5,
                         "bHasRayTracingGeometry not Implented "); // bHasRayTracingGeometry skip 4 bytes
      if (inlined) {
        parseLODBuffer(m_cursor, lodResources);
      } else {
        auto bulkData = m_owner->parseFByteBulkData(m_cursor);
        parseLODBuffer(bulkData.bulkCursor, lodResources);
        m_cursor += 8; // DepthOnlyNumTriangles + Packed
        m_cursor += 4 * 4 + 2 * 4 + 2 * 4 + 5 * 2 * 4;
        // StaticMeshVertexBuffer = 2x int32, 2x bool
        // PositionVertexBuffer = 2x int32
        // ColorVertexBuffer = 2x int32
        // IndexBuffer = int32 + bool
        // ReversedIndexBuffer
        // DepthOnlyIndexBuffer
        // ReversedDepthOnlyIndexBuffer
        // WireframeIndexBuffer
        SAP_ASSERT_UE5_MAX(m_owner, UE5_5, "RawDataHeader = 6x uint32 not Implented ");
      }
    }
    m_cursor += 12;
  }

  void StaticMesh::parseLODBuffer(ByteCursor &cursor, FStaticMeshLODResources &lodResources) {
    auto stripDataFlags = cursor.read<FStripDataFlags>();
    lodResources.positionVertexBuffer = FPositionVertexBuffer::parse(cursor);
    lodResources.vertexBuffer = FStaticMeshVertexBuffer::parse(cursor);
    lodResources.colorVertexBuffer = FColorVertexBuffer::parse(cursor);
    lodResources.indexBuffer = FRawStaticIndexBuffer::parse(cursor);

    if (!stripDataFlags.isClassDataStripped(
            static_cast<uint8_t>(EFStaticMeshLODResourcesStripFlag::CDSF_ReversedIndexBuffer))) {
      lodResources.reversedIndexBuffer = FRawStaticIndexBuffer::parse(cursor);
    }
    lodResources.depthOnlyIndexBuffer = FRawStaticIndexBuffer::parse(cursor);

    if (!stripDataFlags.isClassDataStripped(
            static_cast<uint8_t>(EFStaticMeshLODResourcesStripFlag::CDSF_ReversedIndexBuffer))) {
      lodResources.reversedIndexBuffer = FRawStaticIndexBuffer::parse(cursor);
    }

    if (!stripDataFlags.isEditorDataStripped()) [[unlikely]] {
      lodResources.wireframeIndexBuffer = FRawStaticIndexBuffer::parse(cursor);
    }

    SAP_ASSERT_UE5_MAX(m_owner, UE5_5, "RayTracingResources not Implented ");

    // Skiping FWeightedRandomSampler // areaWeightedSectionSamplers areaWeightedSampler-> +1
    for (int i = 0; i < lodResources.sections.size() + 1; i++) {
      cursor += sizeof(float) * m_cursor.read<int32_t>();
      cursor += sizeof(int32_t) * m_cursor.read<int32_t>();
      cursor += sizeof(float);
    }
  }

  void StaticMesh::parseNaniteResources(FNaniteResources &naniteResources) {
    auto stripDataFlags = m_cursor.read<FStripDataFlags>();
    if (!stripDataFlags.isAudioVisualDataStripped()) {
      naniteResources.resourceFlag = m_cursor.read<NaniteResourceFlag>();
      naniteResources.streamablePages = m_owner->parseFByteBulkData(m_cursor);
      naniteResources.rootPages = m_cursor.readArray<uint8_t>();
      naniteResources.pageStreamingStates = m_cursor.readArray<FPageStreamingState>();
      naniteResources.hierarchyNodes = m_cursor.readArray<FPackedHierarchyNode>();
      naniteResources.rootHierarchyOffset = m_cursor.readArray<uint32_t>();
      naniteResources.pageDependency = m_cursor.readArray<uint32_t>();
      SAP_ASSERT_UE5_MAX(m_owner, UE5_5, "AssemblyTransforms & MeshBounds not Implented "); // Should be UE5_6
      naniteResources.imposterAtlas = m_cursor.readArray<uint16_t>();
      naniteResources.numRootPages = m_cursor.read<uint32_t>();
      naniteResources.positionPrecision = m_cursor.read<int32_t>();
      naniteResources.normalPrecision = m_cursor.read<int32_t>();
      naniteResources.numInputTriangles = m_cursor.read<uint32_t>();
      naniteResources.numInputVertices = m_cursor.read<uint32_t>();
      SAP_ASSERT_UE5_MAX(m_owner, UE5_5, "NumInputMeshes & NumInputTexCoords wont get Read any more"); // as of UE5_6
      naniteResources.numInputMeshes = m_cursor.read<uint16_t>();
      naniteResources.numTexCoords = m_cursor.read<uint16_t>();
      naniteResources.numClusters = m_cursor.read<uint32_t>();
    }
  }

  void StaticMesh::parseInlineDataRepresentation() {
    constexpr uint8_t CardRepresentationDataStripFlag = 2;
    auto stripDataFlags = m_cursor.read<FStripDataFlags>();
    if (!stripDataFlags.isAudioVisualDataStripped() &&
        !stripDataFlags.isClassDataStripped(CardRepresentationDataStripFlag)) {
      for (auto &lod: m_renderData.LODResources) {
        if (m_cursor.readBool()) {
          lod.cardRepresentationData = FCardRepresentationData::parse(m_cursor);
        }
      }
    }
  }

  void StaticMesh::parseStaticMaterials() {
    auto bHasSpeedTreeWind = m_cursor.readBool();
    if (bHasSpeedTreeWind) {

    } else {
      auto count = m_cursor.read<uint32_t>();
      m_materials.reserve(count);
      for (uint32_t i = 0; i < count; i++) {
        m_materials.emplace_back(FStaticMaterial{.materialInterface = static_cast<MaterialInterface*>(
                                                     m_owner->resolveIndex(m_cursor.read<int32_t>())),
                                                 .materialSlotName = m_cursor.readFName(),
                                                 .UVChannelInfo = FMeshUVChannelInfo::parse(m_cursor)});
      }
    }
  }
} // namespace SAP
