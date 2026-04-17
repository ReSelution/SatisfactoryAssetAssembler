//
// Created by oleub on 12.12.25.
//

#pragma once


#include "AssetFileParser.hpp"
#include "byte_cursor.hpp"
#include "exporter.hpp"
#include "core/f_guid.hpp"
#include "core/hash.hpp"
#include "satisfactory/fg_factory.hpp"

#include "ue/properties/property_holder.hpp"
#include "ue/properties/f_property_tag.hpp"

namespace SAA {
    class AssetFileParser;

    class Object : public PropertyHolder {
        friend FGFactory;
        friend Exporter;
        enum class ObjectState : uint8_t {
            NONE,
            PARSING,
            PARSED,
            EXPORTING,
            EXPORTED // Finaler Status
        };

    public:
        explicit Object(std::string_view className = "Object", uint64_t classHash = "Object"_h) : PropertyHolder(
                className), classHash(classHash) {};

        explicit Object(Object *other);

        virtual ~Object() = default;


        [[nodiscard]] std::string_view getClassName() const { return m_className; }

        static constexpr uint64_t StaticHASH = "Object"_h;
        const uint64_t classHash;
        uint64_t getClassHash() { return classHash; }

        void setFileSpan(size_t off, size_t s){
          fileOffset = off;
          fileSize = s;
        }
        void safeParse();

        SC::ChaosEntity safeExport();

        void setOwner(AssetFileParser *parser);

        void setObjName(std::string_view name);

        [[nodiscard]] std::string getFullName() const;

        [[nodiscard]] std::string_view objName() const { return m_objName; }

        [[nodiscard]] std::string_view path() const { return m_owner->path(); }

        [[nodiscard]] Object *outer() const {
            return m_outer;
        }

        void outer(Object *outer) {
            m_outer = outer;
        }

        [[nodiscard]] Object *super() const {
            return m_super;
        }

        void super(Object *super) {
            m_super = super;
        }

        [[nodiscard]] SC::ChaosBumpArena *storage() const {
            return m_owner->storage();
        }

        [[nodiscard]] Object *inner() const {
            return m_inner;
        }

        void setInner(Object *inner) {
            m_inner = inner;
        }

    protected:
        virtual void parse();

        virtual void exprt();

        ByteCursor m_cursor{};
        Object *m_outer = nullptr;
        Object *m_super = nullptr;
        Object *m_inner = nullptr;
        SC::ChaosSpinLock entityCreationLock;
        SC::ChaosEntity entity;


    private:
        size_t fileOffset = 0;
        size_t fileSize = 0;

        std::string_view m_objName;
        FGuid m_objGuid{};

        std::atomic<ObjectState> m_state = ObjectState::NONE;
        std::mutex m_mutex;
        std::mutex m_factoryLock;

        std::condition_variable m_cv;
    };
} 
