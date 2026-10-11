//
// Created by oleub on 12.12.25.
//

#pragma once

#include <ankerl/unordered_dense.h>
#include "asset_object.hpp"
#include "sc/hash/hash.hpp"
#include "sc/memory/monotonic_resource.hpp"

#define SAA_REGISTER_ASSET(TYPE)                                                                                       \
  namespace {                                                                                                          \
    struct TYPE##_Registrar {                                                                                          \
      TYPE##_Registrar() {                                                                                             \
        SAA::ObjectFactory::Register(#TYPE, [](sc::MonotonicResource *storage) -> Object * { return storage->make<TYPE>(); });      \
      }                                                                                                                \
    };                                                                                                                 \
    static TYPE##_Registrar g_##TYPE##_registrar;                                                                      \
  }

namespace SAA {
  class ObjectFactory {

  public:
    using Factory  = Object *(*) (sc::MonotonicResource * storage);
    using denseMap = ankerl::unordered_dense::map<std::string_view, Factory, sc::IdentityHash>;

    static bool Register(std::string_view name, Factory factory) {
      map()[name] = factory;
      return true;
    }

    static Object *Create(std::string_view name, sc::MonotonicResource *storage) {
      auto it = map().find(name);
      return (it != map().end()) ? it->second(storage) : nullptr;
    }

  private:
    static denseMap &map() {
      static denseMap s_map;
      return s_map;
    }
  };
} // namespace SAA
