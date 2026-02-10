//
// Created by oleub on 13.01.26.
//

#pragma once
namespace SAP {
  struct FStaticComponentMaskParameter {
    bool r, g, b, a;

    // static FStaticComponentMaskParameter get(FPropertyValue *val) {
    //   FStaticComponentMaskParameter result{};
    //   if (auto * strct = std::get_if<FStructFallback>(&val->Data)) {
    //     result.r = strct->getOrDefault<bool>(fnv1a("R"), []() { return false; });
    //     result.g = strct->getOrDefault<bool>(fnv1a("G"), []() { return false; });
    //     result.b = strct->getOrDefault<bool>(fnv1a("B"), []() { return false; });
    //     result.a = strct->getOrDefault<bool>(fnv1a("A"), []() { return false; });
    //   }
    //   return result;
    //}
  };
}
