//
// Created by oleub on 30.01.26.
//

#pragma once
#include "fg_descriptor.hpp"
#include "ue/asset_object.hpp"
#include "ue/object_factory.hpp"

namespace SAP {
  class FGItemDescriptor : public FGDescriptor {
    struct Item {
      // Kern-Identität & UI
      static constexpr uint32_t DisplayName = fnv1a("mDisplayName");
      static constexpr uint32_t Description = fnv1a("mDescription");
      static constexpr uint32_t AbbreviatedDisplayName = fnv1a("mAbbreviatedDisplayName");
      static constexpr uint32_t PersistentBigIcon = fnv1a("mPersistentBigIcon");
      static constexpr uint32_t SmallIcon = fnv1a("mSmallIcon");
      static constexpr uint32_t Category = fnv1a("mCategory");
      static constexpr uint32_t MenuPriority = fnv1a("mMenuPriority");

      // Physikalische Eigenschaften (Wichtig für den Flow im Simplex)
      static constexpr uint32_t Form = fnv1a("mForm"); // RF_SOLID, RF_LIQUID, etc.
      static constexpr uint32_t StackSize = fnv1a("mStackSize"); // Enum
      static constexpr uint32_t CachedStackSize = fnv1a("mCachedStackSize");
      static constexpr uint32_t FluidColor = fnv1a("mFluidColor");
      static constexpr uint32_t GasColor = fnv1a("mGasColor");
      static constexpr uint32_t GasType = fnv1a("mGasType");
      static constexpr uint32_t ConveyorMesh = fnv1a("mConveyorMesh");

      // Wirtschaft & Energie (Der Treibstoff für deinen Solver)
      static constexpr uint32_t EnergyValue = fnv1a("mEnergyValue");
      static constexpr uint32_t RadioactiveDecay = fnv1a("mRadioactiveDecay");
      static constexpr uint32_t CanBeDiscarded = fnv1a("mCanBeDiscarded");
      static constexpr uint32_t ResourceDecay = fnv1a("mRadioactiveDecay");

      // Scanning & Exploration
      static constexpr uint32_t ScannerDisplayText = fnv1a("mScannerDisplayText");
      static constexpr uint32_t ScannableType = fnv1a("mScannableType");
      static constexpr uint32_t RequiredSchematicToScan = fnv1a("mRequiredSchematicToScan");
      static constexpr uint32_t ClassToScanFor = fnv1a("mClassToScanFor");
      static constexpr uint32_t ScannerLightColor = fnv1a("mScannerLightColor");
      static constexpr uint32_t ShouldOverrideScannerDisplayText = fnv1a("mShouldOverrideScannerDisplayText");

      // Sonstiges / Meta
      static constexpr uint32_t RememberPickUp = fnv1a("mRememberPickUp");
      static constexpr uint32_t ItemIndex = fnv1a("mItemIndex");
      static constexpr uint32_t IsAlienItem = fnv1a("mIsAlienItem");
      static constexpr uint32_t CustomScannableDetails = fnv1a("mCustomScannableDetails");
      static constexpr uint32_t NeedsPickUpMarker = fnv1a("mNeedsPickUpMarker");
    };

  public:
    FGItemDescriptor(std::string_view classname = "FGItemDescriptor") : FGDescriptor(classname) {
    };

    uint32_t getClassHash() override { return HASH; };

    void parse() override;

    static constexpr uint32_t HASH = fnv1a("FGItemDescriptor");

    private:
    std::optional<FText> mDisplayName;
    std::optional<FText> mDescription;
    std::optional<FText> mAbbreviatedDisplayName;

    std::optional<Object*> mCategory;
    std::optional<Object*> mPersistentBigIcon;
    std::optional<Object*> mSmallIcon;
    std::optional<Object*> mConveyorMesh;

    std::optional<FName> mStackSize;
    std::optional<FName> mForm;
    std::optional<FColor> mFluidColor;
    std::optional<FColor> mGasColor;

    float mEnergyValue;
    float mRadioactiveDecay;
    bool mCanBeDiscarded;
    bool mIsAlienItem;
    int32_t mItemIndex;

    std::optional<FName>mScannableType;
    std::optional<SoftObj>mClassToScanFor;
    std::optional<FText>mScannerDisplayText;
    std::optional<Object*>mRequiredSchematic;

  };

  SAP_REGISTER_ASSET(FGItemDescriptor)
} // namespace SAP
