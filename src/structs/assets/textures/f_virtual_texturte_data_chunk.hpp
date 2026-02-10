//
// Created by oleub on 23.01.26.
//

#pragma once
#include <vector>

#include "io/packages/f_byte_bulk_data.hpp"

 enum class EVirtualTextureCodec : uint8_t
{
  Black,			//Special case codec, always outputs black pixels 0,0,0,0
  OpaqueBlack,	//Special case codec, always outputs opaque black pixels 0,0,0,255
  White,			//Special case codec, always outputs white pixels 255,255,255,255
  Flat,			//Special case codec, always outputs 128,125,255,255 (flat normal map)
  RawGPU,			//Uncompressed data in an GPU-ready format (e.g R8G8B8A8, BC7, ASTC, ...)
  ZippedGPU_DEPRECATED,		//Same as RawGPU but with the data zipped
  Crunch_DEPRECATED,			//Use the Crunch library to compress data
  Max,			// Add new codecs before this entry
};


namespace SAP {
  struct FVirtualTextureDataChunk {
    FByteBulkData bulkData;
    uint32_t byteSize;
    uint32_t codecPayloadSiz;
    std::vector<uint32_t> codecPayloadOffset;
    std::vector<EVirtualTextureCodec> codecType;
  };
}
