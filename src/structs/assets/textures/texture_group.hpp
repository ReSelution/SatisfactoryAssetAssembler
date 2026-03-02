//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>

namespace SAA {
  enum class TextureGroup : uint32_t {
    World,
    WorldNormalMap,
    WorldSpecular,
    Character,
    CharacterNormalMap,
    CharacterSpecular,
    Weapon,
    WeaponNormalMap,
    WeaponSpecular,
    Vehicle,
    VehicleNormalMap,
    VehicleSpecular,
    Cinematic,
    Effects,
    EffectsNotFiltered,
    Skybox,
    UI,
    Lightmap,
    RenderTarget,
    MobileFlattened,
    /** Obsolete - kept for backwards compatibility. */
    ProcBuilding_Face,
    /** Obsolete - kept for backwards compatibility. */
    ProcBuilding_LightMap,
    Shadowmap,
    /** No compression, no mips. */
    ColorLookupTable,
    Terrain_Heightmap,
    Terrain_Weightmap,
    /** Using this TextureGroup triggers special mip map generation code only useful for the BokehDOF post process. */
    Bokeh,
    /** No compression, created on import of a .IES file. */
    IESLightProfile,
    /** Non-filtered, useful for 2D rendering. */
    Pixels2D,
    /** Hierarchical LOD generated textures*/
    HierarchicalLOD,
    /** Impostor Color Textures*/
    Impostor,
    /** Impostor Normal and Depth, use default compression*/
    ImpostorNormalDepth,
    /** 8 bit data stored in textures */
    TG_8BitData,
    /** 16 bit data stored in textures */
    TG_16BitData,
    MAX = 66,
  };
}
