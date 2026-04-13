//
// Created by oleub on 10.01.26.
//

#include "mesh_exporter.hpp"

#include "fastgltf/core.hpp"
#include "ue/static_mesh/static_mesh.hpp"

#include "material/material_exporter.hpp"
LOG_ALIAS(MeshExporterLog, "SAA", "Mesh Exporter")

namespace SAA {
  template<typename T>
  void appendData(std::vector<std::byte> &buffer, const T *data, size_t count) {
    size_t byteSize = count * sizeof(T);
    size_t currentSize = buffer.size();
    buffer.resize(currentSize + byteSize);
    std::memcpy(buffer.data() + currentSize, data, byteSize);
  }

  template<typename T>
  uint32_t createAccessor(fastgltf::Asset &asset, std::vector<std::byte> &buffer, size_t offset, size_t count,
                          std::optional<fastgltf::BufferTarget> target) {
    fastgltf::BufferView view;
    view.bufferIndex = 0;
    view.byteOffset = offset;
    view.byteLength = buffer.size() - offset;
    if (target)
      view.target = *target;

    // Automatische Bestimmung des Strides
    view.byteStride = sizeof(T);

    asset.bufferViews.push_back(std::move(view));

    fastgltf::Accessor accessor;
    accessor.bufferViewIndex = asset.bufferViews.size() - 1;
    accessor.count = count;
    accessor.byteOffset = 0;

    // Mapping von C++ Typen auf glTF Enums
    if constexpr (std::is_same_v<T, FVector3F> || std::is_same_v<T, fastgltf::math::vec<float, 3> >) {
      accessor.type = fastgltf::AccessorType::Vec3;
      accessor.componentType = fastgltf::ComponentType::Float;
    } else if constexpr (std::is_same_v<T, fastgltf::math::vec<float, 2> >) {
      accessor.type = fastgltf::AccessorType::Vec2;
      accessor.componentType = fastgltf::ComponentType::Float;
    } else if constexpr (std::is_same_v<T, uint32_t>) {
      accessor.type = fastgltf::AccessorType::Scalar;
      accessor.componentType = fastgltf::ComponentType::UnsignedInt;
    } else if constexpr (std::is_same_v<T, uint16_t>) {
      accessor.type = fastgltf::AccessorType::Scalar;
      accessor.componentType = fastgltf::ComponentType::UnsignedShort;
    } else if constexpr (std::is_same_v<T, FColor>) {
      accessor.type = fastgltf::AccessorType::Vec4;
      accessor.componentType = fastgltf::ComponentType::UnsignedByte;
    } else {
      static_assert("Unkown Accessor type");
    }

    asset.accessors.emplace_back(std::move(accessor));
    return static_cast<uint32_t>(asset.accessors.size() - 1);
  }

  void alignBuffer(std::vector<std::byte> &buffer) {
    while (buffer.size() % 4 != 0) {
      buffer.push_back(std::byte{0});
    }
  }

  int32_t embedPng(fastgltf::Asset &asset, std::vector<std::byte> &binaryBuffer, const ExportedTextureData &texData) {
    if (texData.data.empty())
      return -1;

    size_t offset = (binaryBuffer.size() + 3) & ~3; // 4-byte align
    binaryBuffer.resize(offset + texData.data.size());
    std::memcpy(binaryBuffer.data() + offset, texData.data.data(), texData.data.size());

    fastgltf::BufferView &view = asset.bufferViews.emplace_back();
    view.bufferIndex = 0;
    view.byteOffset = offset;
    view.byteLength = texData.data.size();
    size_t viewIndex = asset.bufferViews.size() - 1;

    fastgltf::Image &image = asset.images.emplace_back();
    image.name = texData.name;
    image.data = fastgltf::sources::BufferView{viewIndex, fastgltf::MimeType::PNG};

    fastgltf::Texture &texture = asset.textures.emplace_back();
    texture.samplerIndex = 0;
    texture.imageIndex = asset.images.size() - 1;

    return static_cast<int32_t>(asset.textures.size() - 1);
  }

  void MeshExporter::exportAsset(StaticMesh *mesh) {
    if (!mesh || mesh->m_renderData.LODResources.empty()) {
      return;
    }
    collectMesh(mesh);
  }

  void MeshExporter::collectMesh(StaticMesh *mesh) {
    const auto &lod = mesh->m_renderData.LODResources[0];
    exportLOD(lod, mesh);
  }

  void MeshExporter::exportLOD(const FStaticMeshLODResources &lod, StaticMesh *mesh) { exportGLTf(lod, mesh); }

  void MeshExporter::writeIndices(const FStaticMeshLODResources &lod, fastgltf::Asset &asset,
                                  std::vector<std::byte> &binaryBuffer, fastgltf::Primitive &primitive) {
    // --- INDICES ---
    {
      alignBuffer(binaryBuffer);
      size_t offset = binaryBuffer.size();
      auto indices = lod.indexBuffer.indices; // Deine FRawStaticIndexBuffer struct
      // for (size_t i = 0; i < indices.size(); i += 3) { std::swap(indices[i], indices[i + 2]); }
      appendData(binaryBuffer, indices.data(), indices.size());

      auto posAcc = createAccessor<uint32_t>(asset, binaryBuffer, offset, indices.size(),
                                             fastgltf::BufferTarget::ElementArrayBuffer);
      primitive.indicesAccessor = posAcc;
    }
  }

  void MeshExporter::writePostion(const FStaticMeshLODResources &lod, StaticMesh *mesh, fastgltf::Asset &asset,
                                  std::vector<std::byte> &binaryBuffer, fastgltf::Primitive &primitive) {
    {
      alignBuffer(binaryBuffer);
      size_t offset = binaryBuffer.size();
      size_t count = lod.positionVertexBuffer.vertices.size();

      // 1. Vertices konvertieren (UE Z-Up -> glTF Y-Up)
      constexpr float UNIT_SCALE = 0.01f;
      std::vector<FVector3F> convertedVertices;
      convertedVertices.reserve(count);
      for (const auto &v: lod.positionVertexBuffer.vertices) {
        convertedVertices.push_back({v.x * UNIT_SCALE, v.z * UNIT_SCALE, v.y * UNIT_SCALE});
      }
      appendData(binaryBuffer, convertedVertices.data(), count);
      auto posAcc = createAccessor<FVector3F>(asset, binaryBuffer, offset, count, fastgltf::BufferTarget::ArrayBuffer);
      const auto &b = mesh->m_renderData.bounds;
      for (int i = 0; i < 8; ++i) {
        // Erzeugt alle Kombinationen von +/- Extent
        float ux = b.origin.x + ((i & 1) ? b.extent.x : -b.extent.x);
        float uy = b.origin.y + ((i & 2) ? b.extent.y : -b.extent.y);
        float uz = b.origin.z + ((i & 4) ? b.extent.z : -b.extent.z);
        fastgltf::math::vec<double, 3> gltfPoint = {ux * UNIT_SCALE, uz * UNIT_SCALE, -uy * UNIT_SCALE};
        asset.accessors[posAcc].updateBoundsToInclude(gltfPoint);
      }
      primitive.attributes.emplace_back(fastgltf::Attribute{"POSITION", posAcc});
    }
  }

  void MeshExporter::writeUVs(const FStaticMeshLODResources &lod, fastgltf::Asset &asset,
                              std::vector<std::byte> &binaryBuffer, fastgltf::Primitive &primitive) {
    alignBuffer(binaryBuffer);

    int32_t numSets = lod.vertexBuffer.numTextCoords;

    for (int32_t setIdx = 0; setIdx < numSets; ++setIdx) {
      alignBuffer(binaryBuffer);
      size_t offset = binaryBuffer.size();

      for (int32_t i = 0; i < lod.vertexBuffer.numVertices; ++i) {
        const auto &UV = lod.vertexBuffer.uvStorage[i * numSets];

        // glTF UVs: Y-Achse invertieren (1.0 - v)
        fastgltf::math::vec<float, 2> gUV = {UV.u, UV.v};
        appendData(binaryBuffer, &gUV, 1);
      }

      auto posAcc = createAccessor<fastgltf::math::vec<float, 2> >(asset, binaryBuffer, offset,
                                                                   static_cast<size_t>(lod.vertexBuffer.numVertices),
                                                                   fastgltf::BufferTarget::ArrayBuffer);
      std::string attrName = "TEXCOORD_" + std::to_string(setIdx);
      primitive.attributes.emplace_back(fastgltf::Attribute{attrName.c_str(), posAcc});
    }
  }

  void MeshExporter::writeNormals(const FStaticMeshLODResources &lod, fastgltf::Asset &asset,
                                  std::vector<std::byte> &binaryBuffer, fastgltf::Primitive &primitive) {
    alignBuffer(binaryBuffer);
    size_t offset = binaryBuffer.size();

    for (uint32_t i = 0; i < lod.vertexBuffer.numVertices; ++i) {
      // Wir nehmen normals[2] aus dem UVItem (das ist die Normale)
      auto &uvItem = lod.vertexBuffer.UVs[i];
      auto n = uvItem.normals[2];

      // Umrechnen der Koordinaten wie bei POSITION: {x, z, -y}
      // Und wir nutzen deine x(), y(), z() Funktionen!
      fastgltf::math::vec<float, 3> gN = {n.x(), n.z(), n.y()};
      appendData(binaryBuffer, &gN, 1);
    }
    auto posAcc = createAccessor<fastgltf::math::vec<float, 3> >(
      asset, binaryBuffer, offset, lod.vertexBuffer.numVertices, fastgltf::BufferTarget::ArrayBuffer);

    primitive.attributes.emplace_back(fastgltf::Attribute{"NORMAL", posAcc});
  }

  void MeshExporter::writeColors(const FStaticMeshLODResources &lod, fastgltf::Asset &asset,
                                 std::vector<std::byte> &binaryBuffer, fastgltf::Primitive &primitive) {
    alignBuffer(binaryBuffer);
    size_t offset = binaryBuffer.size();
    size_t count = lod.colorVertexBuffer.data.size();

    // Achtung: UE ist oft BGRA, glTF will RGBA. Hier evtl. Swizzling nötig.
    appendData(binaryBuffer, lod.colorVertexBuffer.data.data(), count);

    auto posAcc = createAccessor<FColor>(asset, binaryBuffer, offset, count, fastgltf::BufferTarget::ArrayBuffer);

    primitive.attributes.emplace_back(fastgltf::Attribute{"COLOR_0", posAcc});
  }

  void MeshExporter::writeSections(const FStaticMeshLODResources &lod, fastgltf::Asset &asset,
                                   std::vector<std::byte> &binaryBuffer, fastgltf::Mesh &gltfMesh, StaticMesh *mesh,
                                   fastgltf::Primitive &sharedAttributes) {
    for (auto &section: lod.sections) {
      fastgltf::Primitive primitive;
      primitive.type = fastgltf::PrimitiveType::Triangles;
      primitive.attributes = sharedAttributes.attributes;
      writeSection(lod, section, asset, binaryBuffer, primitive);
      if (section.MaterialIndex < mesh->m_materials.size()) {
        auto matObj = mesh->m_materials[section.MaterialIndex].materialInterface;
        if (matObj) {
          matObj->safeParse();
          auto encoded = MaterialExporter::encodeMaterial(matObj);
          fastgltf::Material gMat;
          gMat.name = encoded.name;

          // --- Base Color Texture ---
          for (int i = 0; i < encoded.baseColors.size(); ++i) {
            int32_t texIdx = embedPng(asset, binaryBuffer, encoded.baseColors[i]);
            if (texIdx != -1) {
              fastgltf::TextureInfo info;
              info.textureIndex = static_cast<size_t>(texIdx);
              info.texCoordIndex = static_cast<size_t>(0);
              gMat.pbrData.baseColorTexture.emplace(std::move(info));
            }
          }

          // --- Normal Map ---
          for (int i = 0; i < encoded.normals.size(); ++i) {
            int32_t texIdx = embedPng(asset, binaryBuffer, encoded.normals[i]);
            if (texIdx != -1) {
              fastgltf::NormalTextureInfo info;
              info.textureIndex = static_cast<size_t>(texIdx);
              info.texCoordIndex = static_cast<size_t>(0);
              gMat.normalTexture.emplace(std::move(info));
            }
          }

          // --- Roughness / Metallic ---
          for (int i = 0; i < encoded.metallicRoughness.size(); ++i) {
            int32_t texIdx = embedPng(asset, binaryBuffer, encoded.metallicRoughness[i]);
            if (texIdx != -1) {
              fastgltf::TextureInfo info;
              info.textureIndex = static_cast<size_t>(texIdx);
              info.texCoordIndex = static_cast<size_t>(0);
              gMat.pbrData.metallicRoughnessTexture.emplace(std::move(info));
            }
          }
          asset.materials.emplace_back(std::move(gMat));
          primitive.materialIndex = asset.materials.size() - 1;
          gltfMesh.primitives.emplace_back(std::move(primitive));
        }
      }
    }
  }

  void MeshExporter::writeSection(const FStaticMeshLODResources &lod, const FStaticMeshSection &section,
                                  fastgltf::Asset &asset, std::vector<std::byte> &binaryBuffer,
                                  fastgltf::Primitive &primitive) {
    alignBuffer(binaryBuffer);
    size_t offset = binaryBuffer.size();
    size_t indexCount = static_cast<size_t>(section.NumTriangles) * 3;
    std::vector<uint32_t> sectionIndices;
    sectionIndices.reserve(indexCount);
    for (size_t i = 0; i < indexCount; i += 3) {
      size_t base = section.FirstIndex + i;
      // Swizzle: Clockwise -> Counter-Clockwise
      sectionIndices.emplace_back(lod.indexBuffer.indices[base]);
      sectionIndices.emplace_back(lod.indexBuffer.indices[base + 1]);
      sectionIndices.emplace_back(lod.indexBuffer.indices[base + 2]);
    }
    appendData(binaryBuffer, sectionIndices.data(), sectionIndices.size());
    primitive.indicesAccessor = createAccessor<uint32_t>(asset, binaryBuffer, offset, sectionIndices.size(),
                                                         fastgltf::BufferTarget::ElementArrayBuffer);
  }

  void MeshExporter::exportGLTf(const FStaticMeshLODResources &lod, StaticMesh *mesh) {
    fastgltf::Asset asset;

    std::vector<std::byte> binaryBuffer;
    fastgltf::Mesh gltfMesh;
    fastgltf::Primitive primitive;
    fastgltf::Sampler &sampler = asset.samplers.emplace_back();
    sampler.magFilter = fastgltf::Filter::Linear;
    sampler.minFilter = fastgltf::Filter::Linear;
    sampler.wrapS = fastgltf::Wrap::Repeat;
    sampler.wrapT = fastgltf::Wrap::Repeat;
    primitive.type = fastgltf::PrimitiveType::Triangles;

    writePostion(lod, mesh, asset, binaryBuffer, primitive);
    writeIndices(lod, asset, binaryBuffer, primitive);

    // --- UVs (TEXCOORD_0) ---
    if (lod.vertexBuffer.numVertices > 0 && !lod.vertexBuffer.uvStorage.empty()) {
      writeUVs(lod, asset, binaryBuffer, primitive);
    }

    // --- NORMALS ---
    if (lod.vertexBuffer.numVertices > 0 && !lod.vertexBuffer.UVs.empty()) {
      writeNormals(lod, asset, binaryBuffer, primitive);
    }

    // --- COLORS (Optional, falls vorhanden) ---
    if (!lod.colorVertexBuffer.data.empty()) {
      writeColors(lod, asset, binaryBuffer, primitive);
    }

    // writeSections(lod, asset, binaryBuffer, gltfMesh, mesh, primitive);

    // gltfMesh.primitives.emplace_back(std::move(primitive));
    asset.meshes.emplace_back(std::move(gltfMesh));

    // --- SCENE & NODES ---
    fastgltf::Scene scene;
    fastgltf::Node node;
    node.meshIndex = 0;
    node.name = "UE_StaticMesh";

    asset.nodes.emplace_back(std::move(node));
    scene.nodeIndices.emplace_back(0);
    asset.scenes.emplace_back(std::move(scene));

    // --- BUFFER ---
    // Der Buffer verweist nur auf die Größe, die Daten kommen beim Export
    fastgltf::Buffer buffer;
    buffer.byteLength = binaryBuffer.size();
    buffer.data = fastgltf::sources::Vector{std::move(binaryBuffer)};
    asset.buffers.emplace_back(std::move(buffer));

    // --- EXPORTIEREN (GLB) ---
    fastgltf::FileExporter exporter;

    std::filesystem::path path = options().getPath() / mesh->path();
    path.replace_extension(".glb");
    checkPath(path);
    auto result = exporter.writeGltfBinary(asset, path, fastgltf::ExportOptions::None);

    if (result != fastgltf::Error::None) {
      MeshExporterLog::err(" Failed to write Gltf binary file:{}", fastgltf::getErrorName(result));
    }
  }
} 
