//
// Created by oleub on 26.02.26.
//
#include "soft_object.hpp"
#include "asset_object.hpp"

namespace SAA {
  Object *SoftObj::resolve(AssetParser *assetParser) {

    if (object) {
      return object;
    }
    parser = assetParser->loadPackage(pkgName.text());
    if (!parser) {
      return nullptr;
    }
    return parser->resolveSoftObj(assetName.text());
  }
}
