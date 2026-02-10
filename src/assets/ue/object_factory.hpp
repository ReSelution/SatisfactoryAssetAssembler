//
// Created by oleub on 12.12.25.
//

#pragma once
#include <string>
#include <unordered_map>

#include "asset_object.hpp"
#include "logger.hpp"

#define SAP_REGISTER_ASSET(TYPE)                                  \
namespace {                                                       \
struct TYPE##_Registrar {                                     \
TYPE##_Registrar() {                                      \
SAP::ObjectFactory::Register(                         \
#TYPE,                                            \
[]()->std::shared_ptr<Object> {        \
return std::make_shared<TYPE>();              \
}                                                 \
);                                                    \
}                                                         \
};                                                            \
static TYPE##_Registrar g_##TYPE##_registrar;                 \
}

namespace SAP {
  class ObjectFactory {
  public:
    using Factory = std::shared_ptr<Object>(*)();

    static bool Register(std::string_view name, Factory factory) {
      s_map[name] = factory;
      return true;
    }

    static std::shared_ptr<Object> Create(std::string_view name) {
      auto it = s_map.find(name);
      return (it != s_map.end()) ? it->second() : nullptr;
    }


  private:
    static inline std::unordered_map<std::string_view, Factory> s_map;
  };
} // SAP
