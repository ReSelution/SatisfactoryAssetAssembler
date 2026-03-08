//
// Created by oleub on 12.12.25.
//

#pragma once
#include <string>
#include <unordered_map>

#include "asset_object.hpp"

#define SAA_REGISTER_ASSET(TYPE)                                                                                       \
  namespace {                                                                                                          \
    struct TYPE##_Registrar {                                                                                          \
      TYPE##_Registrar() {                                                                                             \
        SAA::ObjectFactory::Register(#TYPE, [](SC::ChaosSlab *storage) -> Object * { return storage->make<TYPE>(); });   \
      }                                                                                                                \
    };                                                                                                                 \
    static TYPE##_Registrar g_##TYPE##_registrar;                                                                      \
  }

namespace SAA {
  class ObjectFactory {
  public:
    using Factory = Object *(*) (SC::ChaosSlab * storage);

    static bool Register(std::string_view name, Factory factory) {
      s_map[name] = factory;
      return true;
    }

    static Object *Create(std::string_view name, SC::ChaosSlab *storage) {
      auto it = s_map.find(name);
      return (it != s_map.end()) ? it->second(storage) : nullptr;
    }

  private:
    static inline std::unordered_map<std::string_view, Factory> s_map;
  };
} // namespace SAP
