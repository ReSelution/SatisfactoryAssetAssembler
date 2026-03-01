#pragma once
#include "AssetParser.hpp"
#include "core/name.hpp"
//
// Created by oleub on 19.02.26.
//
namespace SAP {
  class Object;


  struct SoftObj {
    FName pkgName;
    FName assetName;
    std::span<const uint8_t> subPath;
    std::shared_ptr<AssetFileParser> parser;
    Object *object;

    Object *resolve(AssetParser *assetParser);
  };
}
