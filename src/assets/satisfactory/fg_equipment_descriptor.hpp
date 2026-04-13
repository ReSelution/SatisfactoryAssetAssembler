//
// Created by oleub on 24.03.26.
//

#pragma once
#include "fg_descriptor_base.hpp"
#include "object_factory.hpp"

namespace SAA {
  class FGEquipmentDescriptor: public  FGDescriptorBase {
    friend class FGExporter;
    public:
    FGEquipmentDescriptor(const std::string_view classname = "FGEquipmentDescriptor"): FGDescriptorBase(classname) {};

    void parse() override;
  };

  SAA_REGISTER_ASSET(FGEquipmentDescriptor)
} // SAA