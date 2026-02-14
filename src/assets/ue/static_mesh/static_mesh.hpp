//
// Created by oleub on 06.01.26.
//

#pragma once
#include "ue/asset_object.hpp"
#include "ue/object_factory.hpp"

#include "assets/nanite/f_nanit_resources.hpp"
#include "assets/static_mesh/f_static_material.hpp"
#include "assets/static_mesh/f_static_mesh_render_data.hpp"

#include "meshes/mesh_exporter.hpp"

namespace SAP {
  class StaticMesh : public Object {
    friend MeshExporter;

  public:
    StaticMesh(std::string_view classname = "StaticMesh") : Object(classname) {};
    void exportAsset() override;

  protected:
    void parse() override;

  private:
    void parseRenderData();
    void parseLODResource(FStaticMeshLODResources &lodResources);
    void parseLODBuffer(ByteCursor &cursor, FStaticMeshLODResources &lodResources);
    void parseNaniteResources(FNaniteResources &naniteResources);
    void parseInlineDataRepresentation();
    void parseDistanceFieldVolumeData();
    void parseStaticMaterials();

    bool m_cooked{};
    Object* m_bodySetup;
    Object* m_NavCollision;
    FGuid m_lightingGuid{};
    std::vector<Object*> m_sockets{};
    FStaticMeshRenderData m_renderData{};
    std::vector<FStaticMaterial> m_materials{};
  };

  SAP_REGISTER_ASSET(StaticMesh)
} // namespace SAP
