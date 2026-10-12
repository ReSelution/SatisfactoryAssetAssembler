//
// Created by oleub on 26.02.26.
//
#include "soft_object.hpp"
#include "asset_object.hpp"

namespace SAA {
  std::pair<std::shared_ptr<AssetFileParser>, Object *> SoftObj::resolve(AssetParser *assetParser) const {
    auto parser = assetParser->loadPackage(pkgName.text());
    if (!parser) {
      return {};
    }
    Object *object = parser->resolveSoftObj(assetName.text());
    if (object == nullptr) {
      return {};
    }
    return {std::move(parser), object};
  }
} // namespace SAA
