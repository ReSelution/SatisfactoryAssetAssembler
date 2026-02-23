//
// Created by oleub on 30.01.26.
//

#pragma once
#include "simdjson.h"

#include "ue/asset_object.hpp"
#include "ue/object_factory.hpp"

namespace SAP {
  class FGItemDescriptorBiomass : public Object {
    struct Biomass {
      // Die Klassiker (schon bekannt, aber hier gelistet für die Vollständigkeit)
      static constexpr uint32_t DisplayName                 = fnv1a("mDisplayName");
      static constexpr uint32_t Description                 = fnv1a("mDescription");
      static constexpr uint32_t AbbreviatedDisplayName      = fnv1a("mAbbreviatedDisplayName");

      static constexpr uint32_t Category                    = fnv1a("mCategory");
      static constexpr uint32_t EnergyValue                 = fnv1a("mEnergyValue");
      static constexpr uint32_t StackSize                   = fnv1a("mStackSize");
      static constexpr uint32_t Form                        = fnv1a("mForm");

      // UI & Assets
      static constexpr uint32_t PersistentBigIcon           = fnv1a("mPersistentBigIcon");
      static constexpr uint32_t SmallIcon                   = fnv1a("mSmallIcon");
      static constexpr uint32_t ConveyorMesh                = fnv1a("mConveyorMesh");
      static constexpr uint32_t FluidColor                  = fnv1a("mFluidColor");

      // Meta (die du als irrelevant markiert hast)
      static constexpr uint32_t CachedStackSize             = fnv1a("mCachedStackSize");
      static constexpr uint32_t RememberPickUp              = fnv1a("mRememberPickUp");
    };
  public:
    FGItemDescriptorBiomass(std::string_view classname = "FGItemDescriptorBiomass") : Object(classname) {}

    void parse() override;
#
    uint32_t getClassHash() override { return HASH; };

    static constexpr uint32_t HASH = fnv1a("FGItemDescriptorBiomass");

  private:
    std::optional<FText> mDisplayName{};
    std::optional<FText> mDescription{};
    std::optional<FText> mAbbreviatedDisplayName{};
    std::optional<FColor> mFluidColor;

    std::optional<Object*> mCategory {};
    std::optional<Object*> mPersistentBigIcon {};
    std::optional<Object*> mSmallIcon {};
    std::optional<Object*> mConveyorMesh {};
    std::optional<FName> mStackSize{};
    std::optional<FName> mForm{};

    float mEnergyValue{};


  };
  SAP_REGISTER_ASSET(FGItemDescriptorBiomass)
} // namespace SAP
