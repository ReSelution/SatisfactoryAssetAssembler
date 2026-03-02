//
// Created by oleub on 10.01.26.
//

#pragma once
#include "exporter.hpp"

#include "assets/static_mesh/f_static_mesh_LOD_resources.hpp"

#include "fastgltf/types.hpp"

namespace SAA {
  class StaticMesh;

  class MeshExporter : public Exporter {
  public:
    MeshExporter() : Exporter() {};
    static void exportAsset(StaticMesh *mesh);

  private:
    static void collectMesh(StaticMesh *mesh);
    static void exportLOD(const FStaticMeshLODResources &lod, StaticMesh *mesh);
    static void writeIndices(const FStaticMeshLODResources &lod, fastgltf::Asset &asset,
                             std::vector<std::byte> &binaryBuffer, fastgltf::Primitive &primitive);
    static void writePostion(const FStaticMeshLODResources &lod, StaticMesh *mesh, fastgltf::Asset &asset,
                             std::vector<std::byte> &binaryBuffer, fastgltf::Primitive &primitive);
    static void writeUVs(const FStaticMeshLODResources &lod, fastgltf::Asset &asset,
                         std::vector<std::byte> &binaryBuffer, fastgltf::Primitive &primitive);
    static void writeNormals(const FStaticMeshLODResources &lod, fastgltf::Asset &asset,
                             std::vector<std::byte> &binaryBuffer, fastgltf::Primitive &primitive);
    static void writeColors(const FStaticMeshLODResources &lod, fastgltf::Asset &asset,
                            std::vector<std::byte> &binaryBuffer, fastgltf::Primitive &primitive);
    static void writeSections(const FStaticMeshLODResources &lod, fastgltf::Asset &asset,
                              std::vector<std::byte> &binaryBuffer, fastgltf::Mesh &gltfMesh, StaticMesh *mesh,
                              fastgltf::Primitive &sharedAttributes);
    static void writeSection(const FStaticMeshLODResources &lod, const FStaticMeshSection &section,
                             fastgltf::Asset &asset, std::vector<std::byte> &binaryBuffer,
                             fastgltf::Primitive &primitive);

    static void exportGLTf(const FStaticMeshLODResources &lod, StaticMesh *mesh);
  };
} // namespace SAP
