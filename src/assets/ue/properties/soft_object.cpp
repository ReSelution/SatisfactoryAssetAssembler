//
// Created by oleub on 26.02.26.
//
#include "soft_object.hpp"
#include "ue/asset_object.hpp"

namespace SAP {
  Object *SoftObj::resolve(AssetParser *assetParser) {

    if (object) {
      return object;
    }
    auto pkg = FPackageId::fromName(pkgName.text());
    parser = assetParser->loadPackage(pkg);
    if (!parser) {
      return nullptr;
    }
    return parser->resolveSoftObj(assetName.text());
  }
}
