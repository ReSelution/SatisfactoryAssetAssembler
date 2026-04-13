//
// Created by oleub on 01.03.26.
//

#pragma once
#include "../fg_descriptor_base.hpp"

namespace SAA {
  struct ItemDescriptor;

  class FGItemBase : public FGDescriptorBase {
    friend class FGExporter;
  protected:
    struct Item {
        static constexpr uint64_t AbbreviatedDisplayName = "mAbbreviatedDisplayName"_h;

        static constexpr uint64_t PersistentBigIcon = "mPersistentBigIcon"_h;
        static constexpr uint64_t SmallIcon = "mSmallIcon"_h;
        static constexpr uint64_t Category = "mCategory"_h;
        static constexpr uint64_t MenuPriority = "mMenuPriority"_h;

        static constexpr uint64_t Form = "mForm"_h; // RF_SOLID, RF_LIQUID, etc.
        static constexpr uint64_t StackSize = "mStackSize"_h; // Enum
        static constexpr uint64_t CachedStackSize = "mCachedStackSize"_h;
        static constexpr uint64_t FluidColor = "mFluidColor"_h;
        static constexpr uint64_t GasColor = "mGasColor"_h;
        static constexpr uint64_t GasType = "mGasType"_h;
        static constexpr uint64_t ConveyorMesh = "mConveyorMesh"_h;

        static constexpr uint64_t EnergyValue = "mEnergyValue"_h;
        static constexpr uint64_t RadioactiveDecay = "mRadioactiveDecay"_h;
        static constexpr uint64_t CanBeDiscarded = "mCanBeDiscarded"_h;
        static constexpr uint64_t ResourceDecay = "mRadioactiveDecay"_h; // Hinweis: Identisch mit RadioactiveDecay im Original

        static constexpr uint64_t ScannerDisplayText = "mScannerDisplayText"_h;
        static constexpr uint64_t ScannableType = "mScannableType"_h;
        static constexpr uint64_t RequiredSchematicToScan = "mRequiredSchematicToScan"_h;
        static constexpr uint64_t ClassToScanFor = "mClassToScanFor"_h;
        static constexpr uint64_t ScannerLightColor = "mScannerLightColor"_h;
        static constexpr uint64_t ShouldOverrideScannerDisplayText = "mShouldOverrideScannerDisplayText"_h;

        static constexpr uint64_t RememberPickUp = "mRememberPickUp"_h;
        static constexpr uint64_t ItemIndex = "mItemIndex"_h;
        static constexpr uint64_t IsAlienItem = "mIsAlienItem"_h;
        static constexpr uint64_t CustomScannableDetails = "mCustomScannableDetails"_h;
        static constexpr uint64_t NeedsPickUpMarker = "mNeedsPickUpMarker"_h;

        static constexpr uint64_t CompassTexture = "mCompassTexture"_h;
        static constexpr uint64_t PingColor = "mPingColor"_h;

        static constexpr uint64_t DepositMesh = "mDepositMesh"_h;
        static constexpr uint64_t DepositMaterial = "mDepositMaterial"_h;
        static constexpr uint64_t DecalMaterial = "mDecalMaterial"_h;

        static constexpr uint64_t FactoryMiningParticle = "mFactoryMiningParticle"_h;
        static constexpr uint64_t ManualMiningParticle = "mManualMiningParticle"_h;
        static constexpr uint64_t ManualMiningAudioName = "mManualMiningAudioName"_h;
        static constexpr uint64_t DestroyedParticle = "mDestroyedParticle"_h;
    };

  public:
    explicit FGItemBase(const std::string_view className = "FGItemBase", uint64_t clsHash = StaticHASH) : FGDescriptorBase(className, clsHash) {
    }

    explicit FGItemBase(Object *object);

  protected:
    void parse() override;

    std::optional<FText> mDisplayName;
    std::optional<FText> mDescription;
    std::optional<FText> mAbbreviatedDisplayName;

    std::optional<Object *> mCategory;
    std::optional<Object *> mPersistentBigIcon;
    std::optional<Object *> mSmallIcon;
    std::optional<Object *> mConveyorMesh;
    std::optional<FName> mStackSize;

    std::optional<FName> mForm;
    std::optional<FColor> mFluidColor;
    std::optional<FColor> mGasColor;
    std::optional<FName> mGasType;

    float mEnergyValue{};
    float mRadioactiveDecay{};
    bool mCanBeDiscarded{};
    bool mIsAlienItem{};
    int32_t mItemIndex{};

    std::optional<FName> mScannableType;
    std::optional<SoftObj> mClassToScanFor;
    std::optional<FText> mScannerDisplayText;
    std::optional<Object *> mRequiredSchematic;



    std::optional<Object *> mCompassTexture;
    std::optional<Object *> mDepositMesh;
    std::optional<FLinearColor> mPingColor;
  };
} // SAP
