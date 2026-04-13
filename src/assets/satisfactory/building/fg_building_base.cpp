//
// Created by oleub on 12.03.26.
//

#include "fg_building_base.hpp"

#include "logger/chaos_logger.hpp"
LOG_ALIAS(buildingLog, "SAA", "Buildings")

namespace SAA {
  FGBuildingBase::FGBuildingBase(Object *object) : FGDescriptorBase(object) {
  }

  void FGBuildingBase::parse() {
    FGDescriptorBase::parse();

#ifndef NDEBUG
    markHandledDBG();
#endif

    mInventorySizeX = get<int32_t>(Building::InventorySizeX).value_or(mInventorySizeX);
    mInventorySizeX = get<int8_t>(Building::StorageSizeX).value_or(mInventorySizeX);

    mInventorySizeY = get<int32_t>(Building::InventorySizeY).value_or(mInventorySizeY);
    mInventorySizeY = get<int8_t>(Building::StorageSizeY).value_or(mInventorySizeY);

    mPowerInfo = get<Object *>(Building::PowerInfo).value_or(mPowerInfo);
    mPowerConsumption = get<float>(Building::PowerConsumption).value_or(mPowerConsumption);
    mPowerConsumptionExp = get<float>(Building::PowerConsumptionExponent).value_or(mPowerConsumptionExp);
    mPowerProduction = get<float>(Building::PowerProduction).value_or(mPowerProduction);

    mVariablePowerProductionConstant = get<float>(Building::VariablePowerProductionConstant).value_or(
      mVariablePowerProductionConstant);

    mPowerConsumptionCurve = get<Object *>(Building::PowerConsumptionCurve).value_or(mPowerConsumptionCurve);

    mProductionShardBoostMultiplier = get<float>(Building::ProductionShardBoostMultiplier).value_or(
      mProductionShardBoostMultiplier);
    mProductionShardSlotSize = get<int32_t>(Building::ProductionShardSlotSize).value_or(mProductionShardSlotSize);

    mAllowedResources = getArray<Object *>(Building::AllowedResources);
    mAllowedResourceForms = getArray<FName>(Building::AllowedResourceForms);

  }

#ifndef NDEBUG
  void FGBuildingBase::markHandledDBG() const {
    // --- ENGINE & TICKING (Vom Savegame/Runtime gesteuert) ---
    DBG_MARK_HANDLED(m_className, Building::UberGraphFrame);
    DBG_MARK_HANDLED(m_className, Building::NetDormancy);
    DBG_MARK_HANDLED(m_className, Building::PrimaryActorTick);
    DBG_MARK_HANDLED(m_className, Building::MinNetUpdateFrequency);
    DBG_MARK_HANDLED(m_className, Building::NetUpdateFrequency);
    DBG_MARK_HANDLED(m_className, Building::NetPriority);
    DBG_MARK_HANDLED(m_className, Building::NetCullDistanceSquared);
    DBG_MARK_HANDLED(m_className, Building::AlwaysRelevant);
    DBG_MARK_HANDLED(m_className, Building::FirstRun);
    DBG_MARK_HANDLED(m_className, Building::LastEditedBy);

    // --- RUNTIME STATE & DYNAMIC DATA (Wird im Savegame überschrieben) ---
    DBG_MARK_HANDLED(m_className, Building::CurrentPotential); // Übertaktung steht im Save!
    DBG_MARK_HANDLED(m_className, Building::ActivePowerConsumption); // Live-Wert
    DBG_MARK_HANDLED(m_className, Building::CurrentPotential); // Alias/Duplicate
    DBG_MARK_HANDLED(m_className, Building::ToggleMiningStateHandle_0);
    DBG_MARK_HANDLED(m_className, Building::IsFullBlast);
    DBG_MARK_HANDLED(m_className, Building::LightningTimer);
    DBG_MARK_HANDLED(m_className, Building::PistonAudioTimer);
    DBG_MARK_HANDLED(m_className, Building::AudioCounterTimer);
    DBG_MARK_HANDLED(m_className, Building::NotifyNameREferences);
    DBG_MARK_HANDLED(m_className, Building::newCustomizationData); // Farben/Skins kommen aus Save

    // --- VISUALS & AUDIO (Statische Asset-Referenzen, uninteressant für Logik) ---
    DBG_MARK_HANDLED(m_className, Building::AkEvent);
    DBG_MARK_HANDLED(m_className, Building::Play_AOLayeredLoopingEvent);
    DBG_MARK_HANDLED(m_className, Building::Stop_AOLayeredLoopingEvent);
    DBG_MARK_HANDLED(m_className, Building::Play_AOPowerInputLoopingEvent);
    DBG_MARK_HANDLED(m_className, Building::StartAudioEvent);
    DBG_MARK_HANDLED(m_className, Building::StopHeadLiftSound);
    DBG_MARK_HANDLED(m_className, Building::PipelineEngineSound);
    DBG_MARK_HANDLED(m_className, Building::PipelinePistonSound);
    DBG_MARK_HANDLED(m_className, Building::StopPipelineEngineSound);
    DBG_MARK_HANDLED(m_className, Building::StopPipelinePistonSound);
    DBG_MARK_HANDLED(m_className, Building::StartRattleSoundEvent);
    DBG_MARK_HANDLED(m_className, Building::StopRattleSoundEvent);
    DBG_MARK_HANDLED(m_className, Building::SplineAudioEvent);
    DBG_MARK_HANDLED(m_className, Building::HeadLiftSound);
    DBG_MARK_HANDLED(m_className, Building::SFXLocation);
    DBG_MARK_HANDLED(m_className, Building::SFXSockets);
    DBG_MARK_HANDLED(m_className, Building::AO_Sweetener_Switch);
    DBG_MARK_HANDLED(m_className, Building::AOAttenuationScalingFactor);
    DBG_MARK_HANDLED(m_className, Building::AOLayerZOffset);
    DBG_MARK_HANDLED(m_className, Building::BuildEffectSpeed);
    DBG_MARK_HANDLED(m_className, Building::HideOnBuildEffectStart);
    DBG_MARK_HANDLED(m_className, Building::AnimationRate);
    DBG_MARK_HANDLED(m_className, Building::ForceLegacyBuildEffect);
    DBG_MARK_HANDLED(m_className, Building::CurrentColorVFX);
    DBG_MARK_HANDLED(m_className, Building::CurrentColor_VFX);
    DBG_MARK_HANDLED(m_className, Building::VFX_LaserSubtract);
    DBG_MARK_HANDLED(m_className, Building::Beam_TargetLoc);
    DBG_MARK_HANDLED(m_className, Building::VFX_SubVector);
    DBG_MARK_HANDLED(m_className, Building::VFX_SummerSloop_Loc_Offset);
    DBG_MARK_HANDLED(m_className, Building::FireLaserEvent);
    DBG_MARK_HANDLED(m_className, Building::MainLaserEvent);
    DBG_MARK_HANDLED(m_className, Building::StopLaserEvent);
    DBG_MARK_HANDLED(m_className, Building::LaserSocketNames);
    DBG_MARK_HANDLED(m_className, Building::LightningEndLoc);
    DBG_MARK_HANDLED(m_className, Building::LightningEndSocket);
    DBG_MARK_HANDLED(m_className, Building::LightningEndVector);
    DBG_MARK_HANDLED(m_className, Building::LightningStartLoc);
    DBG_MARK_HANDLED(m_className, Building::LightningStartSocket);
    DBG_MARK_HANDLED(m_className, Building::LightningStartVector);
    DBG_MARK_HANDLED(m_className, Building::Timeline_Offset_942DA00C47315AC741095991E04356D7);

    // --- COLLISION & PHYSICS (Statisch im Asset definiert) ---
    DBG_MARK_HANDLED(m_className, Building::CollisionComponent);
    DBG_MARK_HANDLED(m_className, Building::CollisionProxyMesh);
    DBG_MARK_HANDLED(m_className, Building::ClearanceThickness);
    DBG_MARK_HANDLED(m_className, Building::ClearanceData);
    DBG_MARK_HANDLED(m_className, Building::PhysicalMaterial);
    DBG_MARK_HANDLED(m_className, Building::JumpForceCharacter);
    DBG_MARK_HANDLED(m_className, Building::JumpForcePhysics);
    DBG_MARK_HANDLED(m_className, Building::OcclusionBoxInfo);
    DBG_MARK_HANDLED(m_className, Building::OcclusionShape);
    DBG_MARK_HANDLED(m_className, Building::AffectsOcclusion);

    // --- MISC UNINTERESTING ---
    DBG_MARK_HANDLED(m_className, Building::RootComponent);
    DBG_MARK_HANDLED(m_className, Building::AddToSignificanceManager);
    DBG_MARK_HANDLED(m_className, Building::SignificanceRange);
    DBG_MARK_HANDLED(m_className, Building::GainSignificanceDistance);
    DBG_MARK_HANDLED(m_className, Building::IsTickRateManaged);
    DBG_MARK_HANDLED(m_className, Building::TickExponent);
    DBG_MARK_HANDLED(m_className, Building::ManagedByLightweightBuildableSubsystem);
    DBG_MARK_HANDLED(m_className, Building::CanContainLightweightInstances);

    DBG_MARK_HANDLED(m_className, Building::HologramClass);
    DBG_MARK_HANDLED(m_className, Building::HologramOverrides);
    DBG_MARK_HANDLED(m_className, Building::ShouldModifyWorldGrid);
    DBG_MARK_HANDLED(m_className, Building::SnapOnly0);
    DBG_MARK_HANDLED(m_className, Building::DisableAttachmentSnapOn);
    DBG_MARK_HANDLED(m_className, Building::DisableSnapOn);
    DBG_MARK_HANDLED(m_className, Building::MinimumDepthForPlacement);
    DBG_MARK_HANDLED(m_className, Building::UsesDistanceForZooping);
    DBG_MARK_HANDLED(m_className, Building::OffsetFromPortalSurface);
    DBG_MARK_HANDLED(m_className, Building::ClearanceData);
    DBG_MARK_HANDLED(m_className, Building::ClearanceThickness);
    DBG_MARK_HANDLED(m_className, Building::BlockVisualizationMesh);

    DBG_MARK_HANDLED(m_className, Building::Timeline_Offset_942DA00C47315AC741095991E04356D7);
    DBG_MARK_HANDLED(m_className, Building::CentralStorageSwitchEvent);
    DBG_MARK_HANDLED(m_className, Building::EventType);
    DBG_MARK_HANDLED(m_className, Building::MiniGameLocation);
    DBG_MARK_HANDLED(m_className, Building::CalendarLocation);
    DBG_MARK_HANDLED(m_className, Building::CharacterCustomizationLocation);
    DBG_MARK_HANDLED(m_className, Building::PioneerPottyLocation);
    DBG_MARK_HANDLED(m_className, Building::PioneerPottyClass);
    DBG_MARK_HANDLED(m_className, Building::Shrine_location);
    DBG_MARK_HANDLED(m_className, Building::WorkBenchLocation);
    DBG_MARK_HANDLED(m_className, Building::HubTerminalLocation);
    DBG_MARK_HANDLED(m_className, Building::StorageLocation);
    DBG_MARK_HANDLED(m_className, Building::DroneDockingLocationLocal);
    DBG_MARK_HANDLED(m_className, Building::DroneDockingStartLocationLocal);

    DBG_MARK_HANDLED(m_className, Building::BuildableSparseDataCDO);
    DBG_MARK_HANDLED(m_className, Building::ConveyorLiftSparesDataCDO);
    DBG_MARK_HANDLED(m_className, Building::ElevatorSparseDataCDO);
    DBG_MARK_HANDLED(m_className, Building::RailroadSignalSparesDataCDO);
    DBG_MARK_HANDLED(m_className, Building::ABClass);

    DBG_MARK_HANDLED(m_className, Building::ParticleMap);
    DBG_MARK_HANDLED(m_className, Building::StartVector_VFX_Large_End);
    DBG_MARK_HANDLED(m_className, Building::StartVector_VFX_Large_Start);
    DBG_MARK_HANDLED(m_className, Building::StartVector_VFX_Medium_End);
    DBG_MARK_HANDLED(m_className, Building::StartVector_VFX_Medium_Start);
    DBG_MARK_HANDLED(m_className, Building::StartVector_VFX_Small_End);
    DBG_MARK_HANDLED(m_className, Building::StartVector_VFX_Small_Start);
    DBG_MARK_HANDLED(m_className, Building::TrajectoryMeshRotation);
    DBG_MARK_HANDLED(m_className, Building::TrajectoryMeshScale);
    DBG_MARK_HANDLED(m_className, Building::PortalFluctuation);
    DBG_MARK_HANDLED(m_className, Building::BlendExp);

    DBG_MARK_HANDLED(m_className, Building::RepresentationText);
    DBG_MARK_HANDLED(m_className, Building::MapText);
    DBG_MARK_HANDLED(m_className, Building::StorageText);
    DBG_MARK_HANDLED(m_className, Building::OccupiedText);
    DBG_MARK_HANDLED(m_className, Building::MamFreeText);
    DBG_MARK_HANDLED(m_className, Building::MamOccupiedText);
    DBG_MARK_HANDLED(m_className, Building::WorkBenchFree);
    DBG_MARK_HANDLED(m_className, Building::WorkBenchOccupied);
    DBG_MARK_HANDLED(m_className, Building::WidgetClass);
    DBG_MARK_HANDLED(m_className, Building::InteractWidgetSoftClass);

    DBG_MARK_HANDLED(m_className, Building::MaxRenderDistance);
    DBG_MARK_HANDLED(m_className, Building::DepthTraceOriginOffset);
    DBG_MARK_HANDLED(m_className, Building::EmissiveOnlySignMaterial);
    DBG_MARK_HANDLED(m_className, Building::EngineInterpDuration);
    DBG_MARK_HANDLED(m_className, Building::EstimatedMaximumPowerConsumption);
    DBG_MARK_HANDLED(m_className, Building::EstimatedMininumPowerConsumption);
    DBG_MARK_HANDLED(m_className, Building::FactoryTickFunction);
    DBG_MARK_HANDLED(m_className, Building::FreightCargoType);
    DBG_MARK_HANDLED(m_className, Building::GeneratorVisibilityLevels);
    DBG_MARK_HANDLED(m_className, Building::WorldDimensions);
    DBG_MARK_HANDLED(m_className, Building::UpdateFlowTime);
    DBG_MARK_HANDLED(m_className, Building::TimeToSwapLoadVisibility);
    DBG_MARK_HANDLED(m_className, Building::TimeToSwapUnloadVisibility);
    DBG_MARK_HANDLED(m_className, Building::SortRules);
    DBG_MARK_HANDLED(m_className, Building::PortalName);

    DBG_MARK_HANDLED(m_className, Building::SAMReference);
    DBG_MARK_HANDLED(m_className, Building::Tags);
    DBG_MARK_HANDLED(m_className, Building::TimeToExecuteCheckAfterItemAdded);
    DBG_MARK_HANDLED(m_className, Building::CanSeeUGCCached);
    DBG_MARK_HANDLED(m_className, Building::CanSeeUgcCached);
    DBG_MARK_HANDLED(m_className, Building::ActivationStartupTime);
    DBG_MARK_HANDLED(m_className, Building::ForceNetUpdateOnRegisterPlayer);
    DBG_MARK_HANDLED(m_className, Building::InteractionRegisterPlayerWithCircuit);
    DBG_MARK_HANDLED(m_className, Building::IsUseable);
    DBG_MARK_HANDLED(m_className, Building::IsOwnedByPlatform);
    DBG_MARK_HANDLED(m_className, Building::ControlledBuildableType);
    DBG_MARK_HANDLED(m_className, Building::RefundLocationComponent);
    DBG_MARK_HANDLED(m_className, Building::AngleLimit);
    DBG_MARK_HANDLED(m_className, Building::AngleOffset);
    DBG_MARK_HANDLED(m_className, Building::MaxNumSortRules);
    DBG_MARK_HANDLED(m_className, Building::SortRules);
    DBG_MARK_HANDLED(m_className, Building::MaxCharacters);
    DBG_MARK_HANDLED(m_className, Building::ScannableDescriptors);
    DBG_MARK_HANDLED(m_className, Building::MustPlaceOnResourceDisqualifier);
    DBG_MARK_HANDLED(m_className, Building::RestrictToNodeType);
    DBG_MARK_HANDLED(m_className, Building::LengthPerCost);
    DBG_MARK_HANDLED(m_className, Building::DefaultLength);
    DBG_MARK_HANDLED(m_className, Building::MaxLength);

    DBG_MARK_HANDLED(m_className, Building::DesignPressure);
    DBG_MARK_HANDLED(m_className, Building::MaxPressure);
    DBG_MARK_HANDLED(m_className, Building::DampeningFactor);
    DBG_MARK_HANDLED(m_className, Building::GenerateTunnelCollision);
    DBG_MARK_HANDLED(m_className, Building::RattleLimit);
    DBG_MARK_HANDLED(m_className, Building::PipeVelocityBoostImpulse);

    DBG_MARK_HANDLED(m_className, Building::ConnectionWidgetClass);
    DBG_MARK_HANDLED(m_className, Building::IndicatorLevelMax);
    DBG_MARK_HANDLED(m_className, Building::TripInformationSampleCount);

    DBG_MARK_HANDLED(m_className, Building::EffectUpdateInterval);
    DBG_MARK_HANDLED(m_className, Building::GrinderInterpDuration);
    DBG_MARK_HANDLED(m_className, Building::MinimumProducingTime);
    DBG_MARK_HANDLED(m_className, Building::MinimumStoppedTime);
    DBG_MARK_HANDLED(m_className, Building::TimeToCompleteLoad);
    DBG_MARK_HANDLED(m_className, Building::TimeToCompleteUnload);

    DBG_MARK_HANDLED(m_className, Building::CircuitType);
    DBG_MARK_HANDLED(m_className, Building::SignTypeDescriptor);
    DBG_MARK_HANDLED(m_className, Building::ShipUpgradeLevel);
    DBG_MARK_HANDLED(m_className, Building::DroneQueueVerticalSeparation);
    DBG_MARK_HANDLED(m_className, Building::ActorRepresentationViewDistance);
    DBG_MARK_HANDLED(m_className, Building::ActorRepresentationFogOfWarRevealRadius);

    DBG_MARK_HANDLED(m_className, Building::ConnectionComponentClass);
    DBG_MARK_HANDLED(m_className, Building::DecoratorClass);
    DBG_MARK_HANDLED(m_className, Building::DroneClass);
    DBG_MARK_HANDLED(m_className, Building::ExtractorClass);
    DBG_MARK_HANDLED(m_className, Building::FactorySkinClass);
    DBG_MARK_HANDLED(m_className, Building::LockerBuildableClass);
    DBG_MARK_HANDLED(m_className, Building::ProjectAssemblyClass);
    DBG_MARK_HANDLED(m_className, Building::SupplementalResourceClass);
    DBG_MARK_HANDLED(m_className, Building::FuelItemClass);
    DBG_MARK_HANDLED(m_className, Building::DefaultFuelClasses);

    DBG_MARK_HANDLED(m_className, Building::BufferInventory);
    DBG_MARK_HANDLED(m_className, Building::CouponInventory);
    DBG_MARK_HANDLED(m_className, Building::FuelInventory);
    DBG_MARK_HANDLED(m_className, Building::InputInventory);
    DBG_MARK_HANDLED(m_className, Building::OutputInventory);
    DBG_MARK_HANDLED(m_className, Building::StorageInventory);
    DBG_MARK_HANDLED(m_className, Building::Inventory);
    DBG_MARK_HANDLED(m_className, Building::ShopInventory);
    DBG_MARK_HANDLED(m_className, Building::InstanceDataCDO);
    DBG_MARK_HANDLED(m_className, Building::LadderVisibilityLevel);
    DBG_MARK_HANDLED(m_className, Building::StorageVisibilityLevel);
    DBG_MARK_HANDLED(m_className, Building::VisibilityComponent);
    DBG_MARK_HANDLED(m_className, Building::VisibilityMeshComponent);
    DBG_MARK_HANDLED(m_className, Building::ActorRepresentationCompassMaterial);
    DBG_MARK_HANDLED(m_className, Building::ActorRepresentationTexture);
    DBG_MARK_HANDLED(m_className, Building::CompassMaterial);
    DBG_MARK_HANDLED(m_className, Building::CompassMaterialInstance);
    DBG_MARK_HANDLED(m_className, Building::Connection);
    DBG_MARK_HANDLED(m_className, Building::Connection0);
    DBG_MARK_HANDLED(m_className, Building::Connection1);
    DBG_MARK_HANDLED(m_className, Building::Connections);
    DBG_MARK_HANDLED(m_className, Building::PlatformConnection0);
    DBG_MARK_HANDLED(m_className, Building::PlatformConnection1);
    DBG_MARK_HANDLED(m_className, Building::PlatformConnections);

    DBG_MARK_HANDLED(m_className, Building::CanChangePotential); // Nur UI-Recht
    DBG_MARK_HANDLED(m_className, Building::CanChangeProductionBoost); // Nur UI-Recht
    DBG_MARK_HANDLED(m_className, Building::CanEverMonitorProductivity); // Internes Statistik-Flag
    DBG_MARK_HANDLED(m_className, Building::OnlyAllowCertainResources); // Logik-Check, die Liste steht in mAllowedResources
    DBG_MARK_HANDLED(m_className, Building::InitialMinSpeedFactor); // Nur für die Anlauf-Animation
    DBG_MARK_HANDLED(m_className, Building::SequenceDuration); // Länge einer Animation
    DBG_MARK_HANDLED(m_className, Building::ExtractorTypeName);

    DBG_MARK_HANDLED(m_className, Building::AlternativeMaterialRecipes); // Liste von Rezept-IDs
    DBG_MARK_HANDLED(m_className, Building::DefaultGeneratorRecipe); // Rezept-ID
    DBG_MARK_HANDLED(m_className, Building::DefaultHubTerminalRecipe); // Rezept-ID
    DBG_MARK_HANDLED(m_className, Building::DefaultStorageRecipe); // Rezept-ID
    DBG_MARK_HANDLED(m_className, Building::DefaultWorkBenchRecipe); // Rezept-ID

    DBG_MARK_HANDLED(m_className, Building::BackLadderComponent); // Sub-Komponente
    DBG_MARK_HANDLED(m_className, Building::FrontLadderComponent); // Sub-Komponente
    DBG_MARK_HANDLED(m_className, Building::HeadMeshComponent); // Sub-Komponente
    DBG_MARK_HANDLED(m_className, Building::SupportMeshInstanceData); // Instanz-Daten (oft leer/redundant)
    DBG_MARK_HANDLED(m_className, Building::DestinationMeshHeightOffset); // Nur ein kleiner Verschub-Wert
    DBG_MARK_HANDLED(m_className, Building::WindDirectionFromTurbine);
    DBG_MARK_HANDLED(m_className, Building::FlowIndicatorClass);
    DBG_MARK_HANDLED(m_className, Building::FlowIndicatorMinimumPipeLength);
    DBG_MARK_HANDLED(m_className, Building::InventoryPotential);
    DBG_MARK_HANDLED(m_className, Building::PowerConnectionComponent);
    DBG_MARK_HANDLED(m_className, Building::TripPowerPerMeterCost);
    DBG_MARK_HANDLED(m_className, Building::TripPowerPerMeterCost);
    DBG_MARK_HANDLED(m_className, Building::TripPowerCost);
    DBG_MARK_HANDLED(m_className, Building::DoesHaveShutdownAnimation);

    DBG_MARK_HANDLED(m_className, Building::DestinationMeshHeightOffset);
    DBG_MARK_HANDLED(m_className, Building::PoleOffset);
    DBG_MARK_HANDLED(m_className, Building::PoleScale);
    DBG_MARK_HANDLED(m_className, Building::TerminalDistanceFromEdge);

    DBG_MARK_HANDLED(m_className, Building::EffectUpdateInterval);
    DBG_MARK_HANDLED(m_className, Building::SequenceDuration);
    DBG_MARK_HANDLED(m_className, Building::InitialMinSpeedFactor);


    //generated
    DBG_MARK_HANDLED(m_className, Building::Meshes);
    DBG_MARK_HANDLED(m_className, Building::TiledMesh);
    DBG_MARK_HANDLED(m_className, Building::ActorRepresentationColor);
    DBG_MARK_HANDLED(m_className, Building::AllowColoring);
    DBG_MARK_HANDLED(m_className, Building::AllowPatterning);
    DBG_MARK_HANDLED(m_className, Building::AngledVariants);
    DBG_MARK_HANDLED(m_className, Building::AngularDepth);
    DBG_MARK_HANDLED(m_className, Building::BaseBoostPercentage);
    DBG_MARK_HANDLED(m_className, Building::BaseMeshComponent);
    DBG_MARK_HANDLED(m_className, Building::BasePowerProduction);
    DBG_MARK_HANDLED(m_className, Building::BellowMesh);
    DBG_MARK_HANDLED(m_className, Building::BottomMesh);
    DBG_MARK_HANDLED(m_className, Building::ButtonMesh);
    DBG_MARK_HANDLED(m_className, Building::CapMesh);
    DBG_MARK_HANDLED(m_className, Building::ColorSlot);
    DBG_MARK_HANDLED(m_className, Building::CustomScaleType);
    DBG_MARK_HANDLED(m_className, Building::CustomSkins);
    DBG_MARK_HANDLED(m_className, Building::DefaultSignMaterial);
    DBG_MARK_HANDLED(m_className, Building::DefaultSwatchCustomizationOverride);
    DBG_MARK_HANDLED(m_className, Building::Depth);
    DBG_MARK_HANDLED(m_className, Building::DestinationMesh);
    DBG_MARK_HANDLED(m_className, Building::Dimensions);
    DBG_MARK_HANDLED(m_className, Building::Elevation);
    DBG_MARK_HANDLED(m_className, Building::ElevatorDataStruct);
    DBG_MARK_HANDLED(m_className, Building::EndCapRotation);
    DBG_MARK_HANDLED(m_className, Building::EndCapTranslation);
    DBG_MARK_HANDLED(m_className, Building::ExtractCycleTime);
    DBG_MARK_HANDLED(m_className, Building::ExtractStartupTime);
    DBG_MARK_HANDLED(m_className, Building::FixtureAngle);
    DBG_MARK_HANDLED(m_className, Building::FlipMeshOnReverse);
    DBG_MARK_HANDLED(m_className, Building::FloorMeshComponent);
    DBG_MARK_HANDLED(m_className, Building::FloorMeshTile);
    DBG_MARK_HANDLED(m_className, Building::FlowLimit);
    DBG_MARK_HANDLED(m_className, Building::FluidNames);
    DBG_MARK_HANDLED(m_className, Building::FluidStackSizeMultiplier);
    DBG_MARK_HANDLED(m_className, Building::FuelLoadAmount);
    DBG_MARK_HANDLED(m_className, Building::FuelSlotSize);
    DBG_MARK_HANDLED(m_className, Building::FuelTransferSpeed);
    DBG_MARK_HANDLED(m_className, Building::Generator1Location);
    DBG_MARK_HANDLED(m_className, Building::Generator2Location);
    DBG_MARK_HANDLED(m_className, Building::HalfMidMesh);
    DBG_MARK_HANDLED(m_className, Building::HeatUpCycleTime);
    DBG_MARK_HANDLED(m_className, Building::HeatUpFuelConsumptionCurve);
    DBG_MARK_HANDLED(m_className, Building::HeatUpPowerConsumptionCurve);
    DBG_MARK_HANDLED(m_className, Building::Height);
    DBG_MARK_HANDLED(m_className, Building::IsDoubleRamp);
    DBG_MARK_HANDLED(m_className, Building::IsEndStop);
    DBG_MARK_HANDLED(m_className, Building::IsFrame);
    DBG_MARK_HANDLED(m_className, Building::IsInverted);
    DBG_MARK_HANDLED(m_className, Building::IsPathSignal);
    DBG_MARK_HANDLED(m_className, Building::IsRoof);
    DBG_MARK_HANDLED(m_className, Building::IsSupport);
    DBG_MARK_HANDLED(m_className, Building::ItemType);
    DBG_MARK_HANDLED(m_className, Building::ItemsPerCycle);
    DBG_MARK_HANDLED(m_className, Building::LadderSegmentMesh);
    DBG_MARK_HANDLED(m_className, Building::LaunchPowerCost);
    DBG_MARK_HANDLED(m_className, Building::LaunchVelocity);
    DBG_MARK_HANDLED(m_className, Building::LauncherBox);
    DBG_MARK_HANDLED(m_className, Building::LauncherMeshComponent);
    DBG_MARK_HANDLED(m_className, Building::LegacySwatch);
    DBG_MARK_HANDLED(m_className, Building::MaxSegmentCount);
    DBG_MARK_HANDLED(m_className, Building::Mesh);
    DBG_MARK_HANDLED(m_className, Building::MeshComponentProxy);
    DBG_MARK_HANDLED(m_className, Building::MeshHeight);
    DBG_MARK_HANDLED(m_className, Building::MeshLength);
    DBG_MARK_HANDLED(m_className, Building::MeshMesh);
    DBG_MARK_HANDLED(m_className, Building::Meshes);
    DBG_MARK_HANDLED(m_className, Building::MidMesh);
    DBG_MARK_HANDLED(m_className, Building::MidMeshLength);
    DBG_MARK_HANDLED(m_className, Building::MidMeshRotation);
    DBG_MARK_HANDLED(m_className, Building::MinFuelToStartProducing);
    DBG_MARK_HANDLED(m_className, Building::MinFuelToStartProducingAfterAbruptStop);
    DBG_MARK_HANDLED(m_className, Building::MovementRate);
    DBG_MARK_HANDLED(m_className, Building::NumArrows);
    DBG_MARK_HANDLED(m_className, Building::OverrideProductionShardSlotSize);
    DBG_MARK_HANDLED(m_className, Building::PoleVariations);
    DBG_MARK_HANDLED(m_className, Building::PortalTravelTimeOverDistance);
    DBG_MARK_HANDLED(m_className, Building::PowerBankCapacity);
    DBG_MARK_HANDLED(m_className, Building::PowerConsumptionPerTeleportOverDistance);
    DBG_MARK_HANDLED(m_className, Building::PowerInputCapacity);
    DBG_MARK_HANDLED(m_className, Building::PowerPoleType);
    DBG_MARK_HANDLED(m_className, Building::PowerStoreCapacity);
    DBG_MARK_HANDLED(m_className, Building::Radius);
    DBG_MARK_HANDLED(m_className, Building::RequiresSupplementalResource);
    DBG_MARK_HANDLED(m_className, Building::ResearchMeshScale);
    DBG_MARK_HANDLED(m_className, Building::ScaleCustomOffset);
    DBG_MARK_HANDLED(m_className, Building::ShelfMesh);
    DBG_MARK_HANDLED(m_className, Building::ShouldTeleportHere);
    DBG_MARK_HANDLED(m_className, Building::SignDrawSize);
    DBG_MARK_HANDLED(m_className, Building::SignProxyPlane);
    DBG_MARK_HANDLED(m_className, Building::SignToSignOffset);
    DBG_MARK_HANDLED(m_className, Building::SignalComponent);
    DBG_MARK_HANDLED(m_className, Building::Size);
    DBG_MARK_HANDLED(m_className, Building::SkeletalMeshSoftPtr);
    DBG_MARK_HANDLED(m_className, Building::Speed);
    DBG_MARK_HANDLED(m_className, Building::SplineComponent);
    DBG_MARK_HANDLED(m_className, Building::SplineMeshMaterial);
    DBG_MARK_HANDLED(m_className, Building::StackTransferSize);
    DBG_MARK_HANDLED(m_className, Building::StackingHeight);
    DBG_MARK_HANDLED(m_className, Building::Stages);
    DBG_MARK_HANDLED(m_className, Building::StairDirection);
    DBG_MARK_HANDLED(m_className, Building::StorageCapacity);
    DBG_MARK_HANDLED(m_className, Building::StorageInventorySize);
    DBG_MARK_HANDLED(m_className, Building::SupplementalLoadAmount);
    DBG_MARK_HANDLED(m_className, Building::SupplementalToPowerRatio);
    DBG_MARK_HANDLED(m_className, Building::SwatchGroup);
    DBG_MARK_HANDLED(m_className, Building::SwitchComponent);
    DBG_MARK_HANDLED(m_className, Building::TerminalHalfDepth);
    DBG_MARK_HANDLED(m_className, Building::TerminalMesh);
    DBG_MARK_HANDLED(m_className, Building::TimeToProduceItem);
    DBG_MARK_HANDLED(m_className, Building::TopMesh);
    DBG_MARK_HANDLED(m_className, Building::TrajectorySplineMesh);
    DBG_MARK_HANDLED(m_className, Building::TransferSpeed);
    DBG_MARK_HANDLED(m_className, Building::WallType);
    DBG_MARK_HANDLED(m_className, Building::WidgetMaterial);
    DBG_MARK_HANDLED(m_className, Building::Width);
    DBG_MARK_HANDLED(m_className, Building::WireMesh);
    DBG_MARK_HANDLED(m_className, Building::CentrifugeDuration);
    DBG_MARK_HANDLED(m_className, Building::mMeshes);

  }


#endif
} // SAA
