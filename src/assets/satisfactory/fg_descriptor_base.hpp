//
// Created by oleub on 14.03.26.
//

#pragma once

#include "asset_object.hpp"
#include "threading/chaos_spin_lock.hpp"

namespace SAA {
    struct Recipe;

    class FGDescriptorBase : public Object {
        friend class FGExporter;

    protected:
        struct Base {
            static constexpr uint64_t DisplayName = "mDisplayName"_h;
            static constexpr uint64_t Description = "mDescription"_h;
            static constexpr uint64_t PersistentBigIcon = "mPersistentBigIcon"_h;
            static constexpr uint64_t Category = "mCategory"_h;
            static constexpr uint64_t SmallIcon = "mSmallIcon"_h;
            static constexpr uint64_t ConveyorMesh = "mConveyorMesh"_h;
            static constexpr uint64_t StackSize = "mStackSize"_h;
        };

    public:
        explicit FGDescriptorBase(const std::string_view className = "FGBase", uint64_t hash = StaticHASH) : Object(className, hash) {
        }


        static constexpr uint64_t StaticHASH = "FGBase"_h;

        explicit FGDescriptorBase(Object *object);


    protected:
        void parse() override;

        void addProducedIn(Recipe *recipe);

        void addUsedIn(Recipe *recipe);

        SC::ChaosSpinLock mProducedInLock;
        SC::ChaosSpinLock mUsedInLock;
        std::pmr::vector<Recipe *> *m_producedIn = nullptr;
        std::pmr::vector<Recipe *> *m_usedIn = nullptr;

        std::optional<FText> mDisplayName;
        std::optional<FText> mDescription;
        std::optional<Object *> mPersistentBigIcon;
        std::optional<Object *> mCategory;
        std::optional<Object *> mSmallIcon;
        std::optional<FName> mStackSize;
        std::optional<Object *> mConveyorMesh;

    };
}
