//
// Created by oleub on 12.12.25.
//

#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <string_view>

#include "AssetFileParser.hpp"
#include "byte_cursor.hpp"
#include "core/f_guid.hpp"
#include "core/flags.hpp"
#include "satisfactory/fg_factory.hpp"

#include "exporter.hpp"
#include "sc/ecs/entity.hpp"
#include "sc/memory/monotonic_resource.hpp"
#include "ue/properties/f_property_tag.hpp"
#include "ue/properties/property_holder.hpp"

namespace SAA {
  class AssetFileParser;

  class Object : public PropertyHolder {
    friend class FGFactory;
    friend class Exporter;
    enum class ObjectState : uint8_t {
      NONE,
      PARSING,
      PARSED,
      EXPORTING,
      EXPORTED, // Finaler Status
      // parse() or exprt() threw. safeParse() and safeExport() throw for every caller from then on.
      FAILED
    };

  public:
    explicit Object(std::string_view className = "Object", uint64_t classHash = "Object"_h) :
        classHash(classHash), m_className(className) {};

    explicit Object(Object *other);

    virtual ~Object() = default;


    [[nodiscard]] std::string_view getClassName() const { return m_className; }

    static constexpr uint64_t StaticHASH = "Object"_h;
    const uint64_t classHash;
    uint64_t getClassHash() const { return classHash; }

    void setFileSpan(size_t off, size_t s) {
      fileOffset = off;
      fileSize   = s;
    }

    // The flags of the export whose data setFileSpan() points to.
    void setObjectFlags(EObjectFlags flags) { m_objectFlags = flags; }

    [[nodiscard]] bool isClassDefaultObject() const {
      return (m_objectFlags & EObjectFlags::RF_ClassDefaultObject) != EObjectFlags::RF_NoFlags;
    }
    void safeParse();

    sc::ecs::Entity safeExport();

    void setOwner(AssetFileParser *parser);

    void setObjName(std::string_view name);

    [[nodiscard]] std::string getFullName() const;

    [[nodiscard]] std::string_view objName() const { return m_objName; }

    [[nodiscard]] std::string_view path() const { return m_owner->path(); }

    [[nodiscard]] Object *outer() const { return m_outer; }

    void outer(Object *outer) { m_outer = outer; }

    [[nodiscard]] Object *super() const { return m_super; }

    void super(Object *super) { m_super = super; }

    [[nodiscard]] sc::MonotonicResource *storage() const { return m_owner->storage(); }

    [[nodiscard]] Object *inner() const { return m_inner; }

    void setInner(Object *inner) { m_inner = inner; }

  protected:
    virtual void parse();

    virtual void exprt();

    ByteCursor m_cursor{};
    Object *m_outer = nullptr;
    Object *m_super = nullptr;
    Object *m_inner = nullptr;
    sc::ecs::Entity entity;
    std::string m_className;

#ifdef SAA_TRACK_PROPERTIES
    [[nodiscard]] std::string_view trackedClassName() const override { return m_className; }
#endif


  private:
    size_t fileOffset = 0;
    size_t fileSize   = 0;
    EObjectFlags m_objectFlags = EObjectFlags::RF_NoFlags;

    std::string_view m_objName;
    FGuid m_objGuid{};

    void publishState(ObjectState state);
    void throwIfFailed() const;

    std::atomic<ObjectState> m_state = ObjectState::NONE;
    std::mutex m_mutex;
    std::mutex m_factoryLock;

    std::condition_variable m_cv;
  };
} // namespace SAA
