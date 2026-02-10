//
// Created by oleub on 11.01.26.
//

#pragma once
#include "version.hpp"

#include "ue/asset_object.hpp"
#include "ue/object_factory.hpp"

namespace SAP {
  class StaticMeshSocket: public Object{};
  SAP_REGISTER_ASSET(StaticMeshSocket)
} // SAP