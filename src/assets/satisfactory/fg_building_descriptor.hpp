//
// Created by oleub on 30.01.26.
//

#pragma once
#include "fg_descriptor.hpp"
#include "ue/asset_object.hpp"
#include "ue/object_factory.hpp"

namespace SAP {
  class FGBuildingDescriptor : public FGDescriptor {
    struct Building { // In class FGBuildingDescriptor
      static constexpr uint32_t Category = fnv1a("mCategory");
      static constexpr uint32_t StackSize = fnv1a("mStackSize");
      static constexpr uint32_t QuickSwitchGroup = fnv1a("mQuickSwitchGroup");
      static constexpr uint32_t PersistentBigIcon = fnv1a("mPersistentBigIcon");
      static constexpr uint32_t CachedStackSize = fnv1a("mCachedStackSize");
      static constexpr uint32_t MenuPriority = fnv1a("mMenuPriority");
      static constexpr uint32_t ConveyorMesh = fnv1a("mConveyorMesh");
      static constexpr uint32_t SmallIcon = fnv1a("mSmallIcon");
      static constexpr uint32_t BuildableClass = fnv1a("mBuildableClass");
      static constexpr uint32_t SubCategories = fnv1a("mSubCategories");
      static constexpr uint32_t UsesDistanceForZooping = fnv1a("mUsesDistanceForZooping");
      static constexpr uint32_t UseDisplayNameAndDescription = fnv1a("mUseDisplayNameAndDescription");
    };
  public:
    FGBuildingDescriptor(std::string classname = "FGBuildingDescriptor") : FGDescriptor(classname) {}

    uint32_t getClassHash() override { return HASH; };

    void parse() override;

    static constexpr uint32_t HASH = fnv1a("FGBuildingDescriptor");

    private:
    std::optional<Object*> mCategory;
    std::optional<FName> mStackSize;
    std::optional<Object*> mPersistentBigIcon;
    std::optional<Object*> mSmallIcon;
    std::optional<Object*> mConveyorMesh;
    Object* mBuildableClass;
    std::span<Object*> mSubCategories;

  };

  SAP_REGISTER_ASSET(FGBuildingDescriptor)
} // namespace SAP
