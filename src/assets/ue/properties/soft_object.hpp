#pragma once
#include <memory>
#include <span>
#include <utility>

#include "AssetParser.hpp"
#include "core/name.hpp"
//
// Created by oleub on 19.02.26.
//
namespace SAA {
  class Object;


  // Owns nothing, like every property value (see f_property_tag.hpp): the package it points into is not kept in
  // here.
  struct SoftObj {
    FName pkgName;
    FName assetName;
    std::span<const uint8_t> subPath;

    // Loads the package and looks the object up in it. The object lives as long as the returned parser is held.
    // Both are empty if the package or the object does not exist.
    [[nodiscard]] std::pair<std::shared_ptr<AssetFileParser>, Object *> resolve(AssetParser *assetParser) const;
  };
} // namespace SAA
