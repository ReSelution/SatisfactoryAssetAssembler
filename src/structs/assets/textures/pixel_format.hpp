//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>

#include "core/hash.hpp"

namespace SAP {
  enum class EPixelFormat : uint32_t {
    PF_Unknown = 0,
    PF_A32B32G32R32F = fnv1a("PF_A32B32G32R32F"),
    PF_B8G8R8A8 = fnv1a("PF_B8G8R8A8"),
    PF_G8 = fnv1a("PF_G8"),
    PF_G16 = fnv1a("PF_G16"),
    PF_DXT1 = fnv1a("PF_DXT1"),
    PF_DXT3 = fnv1a("PF_DXT3"),
    PF_DXT5 = fnv1a("PF_DXT5"),
    PF_UYVY = fnv1a("PF_UYVY"),
    PF_FloatRGB = fnv1a("PF_FloatRGB"),
    PF_FloatRGBA = fnv1a("PF_FloatRGBA"),
    PF_DepthStencil = fnv1a("PF_DepthStencil"),
    PF_ShadowDepth = fnv1a("PF_ShadowDepth"),
    PF_R32_FLOAT = fnv1a("PF_R32_FLOAT"),
    PF_G16R16 = fnv1a("PF_G16R16"),
    PF_G16R16F = fnv1a("PF_G16R16F"),
    PF_G16R16F_FILTER = fnv1a("PF_G16R16F_FILTER"),
    PF_G32R32F = fnv1a("PF_G32R32F"),
    PF_A2B10G10R10 = fnv1a("PF_A2B10G10R10"),
    PF_A16B16G16R16 = fnv1a("PF_A16B16G16R16"),
    PF_D24 = fnv1a("PF_D24"),
    PF_R16F = fnv1a("PF_R16F"),
    PF_R16F_FILTER = fnv1a("PF_R16F_FILTER"),
    PF_BC5 = fnv1a("PF_BC5"),
    PF_V8U8 = fnv1a("PF_V8U8"),
    PF_A1 = fnv1a("PF_A1"),
    PF_FloatR11G11B10 = fnv1a("PF_FloatR11G11B10"),
    PF_A8 = fnv1a("PF_A8"),
    PF_R32_UINT = fnv1a("PF_R32_UINT"),
    PF_R32_SINT = fnv1a("PF_R32_SINT"),
    PF_PVRTC2 = fnv1a("PF_PVRTC2"),
    PF_PVRTC4 = fnv1a("PF_PVRTC4"),
    PF_R16_UINT = fnv1a("PF_R16_UINT"),
    PF_R16_SINT = fnv1a("PF_R16_SINT"),
    PF_R16G16B16A16_UINT = fnv1a("PF_R16G16B16A16_UINT"),
    PF_R16G16B16A16_SINT = fnv1a("PF_R16G16B16A16_SINT"),
    PF_R5G6B5_UNORM = fnv1a("PF_R5G6B5_UNORM"),
    PF_R8G8B8A8 = fnv1a("PF_R8G8B8A8"),
    PF_A8R8G8B8 = fnv1a("PF_A8R8G8B8"),
    PF_BC4 = fnv1a("PF_BC4"),
    PF_R8G8 = fnv1a("PF_R8G8"),
    PF_ATC_RGB = fnv1a("PF_ATC_RGB"),
    PF_ATC_RGBA_E = fnv1a("PF_ATC_RGBA_E"),
    PF_ATC_RGBA_I = fnv1a("PF_ATC_RGBA_I"),
    PF_X24_G8 = fnv1a("PF_X24_G8"),
    PF_ETC1 = fnv1a("PF_ETC1"),
    PF_ETC2_RGB = fnv1a("PF_ETC2_RGB"),
    PF_ETC2_RGBA = fnv1a("PF_ETC2_RGBA"),
    PF_R32G32B32A32_UINT = fnv1a("PF_R32G32B32A32_UINT"),
    PF_R16G16_UINT = fnv1a("PF_R16G16_UINT"),
    PF_ASTC_4x4 = fnv1a("PF_ASTC_4x4"),
    PF_ASTC_6x6 = fnv1a("PF_ASTC_6x6"),
    PF_ASTC_8x8 = fnv1a("PF_ASTC_8x8"),
    PF_ASTC_10x10 = fnv1a("PF_ASTC_10x10"),
    PF_ASTC_12x12 = fnv1a("PF_ASTC_12x12"),
    PF_BC6H = fnv1a("PF_BC6H"),
    PF_BC7 = fnv1a("PF_BC7"),
    PF_R8_UINT = fnv1a("PF_R8_UINT"),
    PF_L8 = fnv1a("PF_L8"),
    PF_XGXR8 = fnv1a("PF_XGXR8"),
    PF_R8G8B8A8_UINT = fnv1a("PF_R8G8B8A8_UINT"),
    PF_R8G8B8A8_SNORM = fnv1a("PF_R8G8B8A8_SNORM"),
    PF_R16G16B16A16_UNORM = fnv1a("PF_R16G16B16A16_UNORM"),
    PF_R16G16B16A16_SNORM = fnv1a("PF_R16G16B16A16_SNORM"),
    PF_PLATFORM_HDR_0 = fnv1a("PF_PLATFORM_HDR_0"),
    PF_PLATFORM_HDR_1 = fnv1a("PF_PLATFORM_HDR_1"),
    PF_PLATFORM_HDR_2 = fnv1a("PF_PLATFORM_HDR_2"),
    PF_NV12 = fnv1a("PF_NV12"),
    PF_R32G32_UINT = fnv1a("PF_R32G32_UINT"),
    PF_ETC2_R11_EAC = fnv1a("PF_ETC2_R11_EAC"),
    PF_ETC2_RG11_EAC = fnv1a("PF_ETC2_RG11_EAC"),
    PF_R8 = fnv1a("PF_R8"),
    PF_B5G5R5A1_UNORM = fnv1a("PF_B5G5R5A1_UNORM"),
    PF_ASTC_4x4_HDR = fnv1a("PF_ASTC_4x4_HDR"),
    PF_ASTC_6x6_HDR = fnv1a("PF_ASTC_6x6_HDR"),
    PF_ASTC_8x8_HDR = fnv1a("PF_ASTC_8x8_HDR"),
    PF_ASTC_10x10_HDR = fnv1a("PF_ASTC_10x10_HDR"),
    PF_ASTC_12x12_HDR = fnv1a("PF_ASTC_12x12_HDR"),
    PF_G16R16_SNORM = fnv1a("PF_G16R16_SNORM"),
    PF_R8G8_UINT = fnv1a("PF_R8G8_UINT"),
    PF_R32G32B32_UINT = fnv1a("PF_R32G32B32_UINT"),
    PF_R32G32B32_SINT = fnv1a("PF_R32G32B32_SINT"),
    PF_R32G32B32F = fnv1a("PF_R32G32B32F"),
    PF_R8_SINT = fnv1a("PF_R8_SINT"),
    PF_R64_UINT = fnv1a("PF_R64_UINT"),
    PF_R9G9B9EXP5 = fnv1a("PF_R9G9B9EXP5"),
    PF_P010 = fnv1a("PF_P010"),

    PF_MAX = fnv1a("PF_MAX")
  };

  inline const char *to_string(EPixelFormat f) {
    switch (f) {
      case EPixelFormat::PF_Unknown:
        return "PF_Unknown";
      case EPixelFormat::PF_A32B32G32R32F:
        return "PF_A32B32G32R32F";
      case EPixelFormat::PF_B8G8R8A8:
        return "PF_B8G8R8A8";
      case EPixelFormat::PF_G8:
        return "PF_G8";
      case EPixelFormat::PF_G16:
        return "PF_G16";
      case EPixelFormat::PF_DXT1:
        return "PF_DXT1";
      case EPixelFormat::PF_DXT3:
        return "PF_DXT3";
      case EPixelFormat::PF_DXT5:
        return "PF_DXT5";
      case EPixelFormat::PF_UYVY:
        return "PF_UYVY";
      case EPixelFormat::PF_FloatRGB:
        return "PF_FloatRGB";
      case EPixelFormat::PF_FloatRGBA:
        return "PF_FloatRGBA";
      case EPixelFormat::PF_DepthStencil:
        return "PF_DepthStencil";
      case EPixelFormat::PF_ShadowDepth:
        return "PF_ShadowDepth";
      case EPixelFormat::PF_R32_FLOAT:
        return "PF_R32_FLOAT";
      case EPixelFormat::PF_G16R16:
        return "PF_G16R16";
      case EPixelFormat::PF_G16R16F:
        return "PF_G16R16F";
      case EPixelFormat::PF_G16R16F_FILTER:
        return "PF_G16R16F_FILTER";
      case EPixelFormat::PF_G32R32F:
        return "PF_G32R32F";
      case EPixelFormat::PF_A2B10G10R10:
        return "PF_A2B10G10R10";
      case EPixelFormat::PF_A16B16G16R16:
        return "PF_A16B16G16R16";
      case EPixelFormat::PF_D24:
        return "PF_D24";
      case EPixelFormat::PF_R16F:
        return "PF_R16F";
      case EPixelFormat::PF_R16F_FILTER:
        return "PF_R16F_FILTER";
      case EPixelFormat::PF_BC5:
        return "PF_BC5";
      case EPixelFormat::PF_V8U8:
        return "PF_V8U8";
      case EPixelFormat::PF_A1:
        return "PF_A1";
      case EPixelFormat::PF_FloatR11G11B10:
        return "PF_FloatR11G11B10";
      case EPixelFormat::PF_A8:
        return "PF_A8";
      case EPixelFormat::PF_R32_UINT:
        return "PF_R32_UINT";
      case EPixelFormat::PF_R32_SINT:
        return "PF_R32_SINT";
      case EPixelFormat::PF_PVRTC2:
        return "PF_PVRTC2";
      case EPixelFormat::PF_PVRTC4:
        return "PF_PVRTC4";
      case EPixelFormat::PF_R16_UINT:
        return "PF_R16_UINT";
      case EPixelFormat::PF_R16_SINT:
        return "PF_R16_SINT";
      case EPixelFormat::PF_R16G16B16A16_UINT:
        return "PF_R16G16B16A16_UINT";
      case EPixelFormat::PF_R16G16B16A16_SINT:
        return "PF_R16G16B16A16_SINT";
      case EPixelFormat::PF_R5G6B5_UNORM:
        return "PF_R5G6B5_UNORM";
      case EPixelFormat::PF_R8G8B8A8:
        return "PF_R8G8B8A8";
      case EPixelFormat::PF_A8R8G8B8:
        return "PF_A8R8G8B8";
      case EPixelFormat::PF_BC4:
        return "PF_BC4";
      case EPixelFormat::PF_R8G8:
        return "PF_R8G8";
      case EPixelFormat::PF_ATC_RGB:
        return "PF_ATC_RGB";
      case EPixelFormat::PF_ATC_RGBA_E:
        return "PF_ATC_RGBA_E";
      case EPixelFormat::PF_ATC_RGBA_I:
        return "PF_ATC_RGBA_I";
      case EPixelFormat::PF_X24_G8:
        return "PF_X24_G8";
      case EPixelFormat::PF_ETC1:
        return "PF_ETC1";
      case EPixelFormat::PF_ETC2_RGB:
        return "PF_ETC2_RGB";
      case EPixelFormat::PF_ETC2_RGBA:
        return "PF_ETC2_RGBA";
      case EPixelFormat::PF_R32G32B32A32_UINT:
        return "PF_R32G32B32A32_UINT";
      case EPixelFormat::PF_R16G16_UINT:
        return "PF_R16G16_UINT";
      case EPixelFormat::PF_ASTC_4x4:
        return "PF_ASTC_4x4";
      case EPixelFormat::PF_ASTC_6x6:
        return "PF_ASTC_6x6";
      case EPixelFormat::PF_ASTC_8x8:
        return "PF_ASTC_8x8";
      case EPixelFormat::PF_ASTC_10x10:
        return "PF_ASTC_10x10";
      case EPixelFormat::PF_ASTC_12x12:
        return "PF_ASTC_12x12";
      case EPixelFormat::PF_BC6H:
        return "PF_BC6H";
      case EPixelFormat::PF_BC7:
        return "PF_BC7";
      case EPixelFormat::PF_R8_UINT:
        return "PF_R8_UINT";
      case EPixelFormat::PF_L8:
        return "PF_L8";
      case EPixelFormat::PF_XGXR8:
        return "PF_XGXR8";
      case EPixelFormat::PF_R8G8B8A8_UINT:
        return "PF_R8G8B8A8_UINT";
      case EPixelFormat::PF_R8G8B8A8_SNORM:
        return "PF_R8G8B8A8_SNORM";
      case EPixelFormat::PF_R16G16B16A16_UNORM:
        return "PF_R16G16B16A16_UNORM";
      case EPixelFormat::PF_R16G16B16A16_SNORM:
        return "PF_R16G16B16A16_SNORM";
      case EPixelFormat::PF_PLATFORM_HDR_0:
        return "PF_PLATFORM_HDR_0";
      case EPixelFormat::PF_PLATFORM_HDR_1:
        return "PF_PLATFORM_HDR_1";
      case EPixelFormat::PF_PLATFORM_HDR_2:
        return "PF_PLATFORM_HDR_2";
      case EPixelFormat::PF_NV12:
        return "PF_NV12";
      case EPixelFormat::PF_R32G32_UINT:
        return "PF_R32G32_UINT";
      case EPixelFormat::PF_ETC2_R11_EAC:
        return "PF_ETC2_R11_EAC";
      case EPixelFormat::PF_ETC2_RG11_EAC:
        return "PF_ETC2_RG11_EAC";
      case EPixelFormat::PF_R8:
        return "PF_R8";
      case EPixelFormat::PF_B5G5R5A1_UNORM:
        return "PF_B5G5R5A1_UNORM";
      case EPixelFormat::PF_ASTC_4x4_HDR:
        return "PF_ASTC_4x4_HDR";
      case EPixelFormat::PF_ASTC_6x6_HDR:
        return "PF_ASTC_6x6_HDR";
      case EPixelFormat::PF_ASTC_8x8_HDR:
        return "PF_ASTC_8x8_HDR";
      case EPixelFormat::PF_ASTC_10x10_HDR:
        return "PF_ASTC_10x10_HDR";
      case EPixelFormat::PF_ASTC_12x12_HDR:
        return "PF_ASTC_12x12_HDR";
      case EPixelFormat::PF_G16R16_SNORM:
        return "PF_G16R16_SNORM";
      case EPixelFormat::PF_R8G8_UINT:
        return "PF_R8G8_UINT";
      case EPixelFormat::PF_R32G32B32_UINT:
        return "PF_R32G32B32_UINT";
      case EPixelFormat::PF_R32G32B32_SINT:
        return "PF_R32G32B32_SINT";
      case EPixelFormat::PF_R32G32B32F:
        return "PF_R32G32B32F";
      case EPixelFormat::PF_R8_SINT:
        return "PF_R8_SINT";
      case EPixelFormat::PF_R64_UINT:
        return "PF_R64_UINT";
      case EPixelFormat::PF_R9G9B9EXP5:
        return "PF_R9G9B9EXP5";
      case EPixelFormat::PF_P010:
        return "PF_P010";
      case EPixelFormat::PF_MAX:
        return "PF_MAX";
      default:
        return "UnknownPixelFormat";
    }
  }
} // namespace SAP
