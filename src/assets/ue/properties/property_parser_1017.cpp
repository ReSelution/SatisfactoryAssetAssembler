//
// Created by oleub on 23.02.26.
//

#include <cstddef>
#include <stdexcept>

#include "AssetParser.hpp"
#include "core/EClassSerializationControlExtension.hpp"
#include "property_parser.hpp"


// Hot path, measured on the test parse of 1017 (28214 files, 21.3 M property values, 15 threads, perf at 2 kHz).
// The run has two parts: about 1.3 s in which all threads work, and a tail of about 0.6 s in which 3 to 5 threads
// decompress the chunks of the big files (GameFileImpl::tryLoad()). Before the chunks were loaded in parallel the
// tail was one thread for 2.4 s and the run took 3.6 s instead of 2.0 s.
// Samples of the whole run: Oodle 73 %, kernel 5 %, own code 20 %. Of the own code:
//   parsePropertyTag() 2.1 %, parsePropertyValue() 2.0 %, PropertyMapBuilder (build/add/resize) 3.0 %,
//   readFName() 1.0 %, parsePropertyTagData() 0.8 %.
// Done, own code went from 21.4 k to 7.3 k samples and the parallel part from 1.85 s to 1.25 s:
// - Property values are no longer destroyed (f_property_tag.hpp) and no longer copied when a container grows.
// - PropertyMap instead of a hash map per holder, and FProperty (104 B) instead of FPropertyTag (232 B, 176 B by now) as what
//   is stored (property_map.hpp).
// - FName is 24 B without the slab pointer, which brought FPropertyValue from 112 B to 72 B.
// - The names of tags and types are hashed once per file when the name map is read (AssetFileParser::nameHash()),
//   no longer for every use: 2.7 M names in all name maps against 25.7 M hashes before. FPropertyTagData is only
//   made for the types that have something in it, 4.6 M of the 11.0 M tags have none. Together 0.3 % fewer
//   instructions in the whole run (118.6 G -> 118.2 G). parsePropertyTag() and parsePropertyTagData() lost about
//   200 samples, about 100 of them show up again at the entry of parsePropertyValue(), whose code did not change.
// property_count as a thread_local is not an issue: the linker turns the access into a plain offset from the
// thread pointer, the binary has no call to __tls_get_addr.
// The "Properties" stat counts parsed values, not distinct ones, and so differs slightly from run to run. Measured
// on the test parse of 1017: 21 583 386 values of 441 266 distinct objects in every run, plus about 1.5 k values
// from 4.5 k to 4.6 k objects that are parsed a second time, with 15 threads as with -j 1 (before imports were made
// on first use: 15.3 k / 9.7 k values). Those sit in imported packages: the parser cache holds them weakly, an import
// whose last holder is gone is created and parsed again (AssetParser::loadPackage()), and whether a holder is still
// there depends on the timing.
namespace SAA {

  thread_local size_t property_count = 0;
  template<>
  void PP6::parse(PropertyHolder *holder, ByteCursor &cursor, sc::MonotonicResource *storage) {
    auto controlByte = cursor.read<EClassSerializationControlExtension>();
    if (hasFlag(controlByte, EClassSerializationControlExtension::OverridableSerializationInformation)) {
      cursor += 1;
    }

    PropertyMapBuilder properties;
    while (true) {
      auto tag = parsePropertyTag(cursor, storage);
      if (tag.Name.isNone()) {

        PropsTroughput::record(property_count);
        property_count = 0;
        holder->m_properties = properties.build(cursor.m_owner->storage());
        return;
      }
      auto final = cursor.tell() + tag.Size;
      tag.Tag = parsePropertyValue(cursor, storage, tag, ReadType::NORMAL, tag.Size);
      cursor = final;
      properties.add(PropertyHolder::propertyKey(cursor.m_owner->nameHash(tag.Name), tag.ArrayIndex), std::move(tag));
    }
  }

  template<>
  FPropertyTag PP6::parsePropertyTag(ByteCursor &cursor, sc::MonotonicResource *storage) {
    int debug = cursor.tell();
    (void) debug;

    auto tag = FPropertyTag{.Name = cursor.readFName()};
    // Logger::logInfo("[Property]", "Property: {} @ {}",tag.Name.text() ,cursor.tell());
    if (tag.Name.isNone()) {
      return tag;
    }

    // The type name is a tree in pre-order. Nodes that do not fit into typeName are still read, Size and Flags
    // follow right behind them; the type is then only known down to the nodes that were kept.
    std::array<FPropertyTypeNameNode, 10> typeName;
    size_t nodeCount = 0;
    int32_t remaining = 1;
    while (remaining > 0) {
      const auto nodeName = cursor.readFName();
      const auto innerCount = cursor.read<int32_t>();
      remaining += innerCount - 1;
      if (nodeCount < typeName.size()) [[likely]] {
        typeName[nodeCount++] = {nodeName, innerCount, cursor.m_owner->nameHash(nodeName)};
      }
    }

    tag.PropertyTypeName = typeName[0].Name;
    tag.PropertyType = static_cast<EPropertyTagType>(typeName[0].NameHash);
    tag.TagData = parsePropertyTagData(storage, tag.PropertyType, std::span{typeName.data(), nodeCount},
                                       tag.PropertyTypeName);
    tag.Size = cursor.read<uint32_t>();
    tag.Flags = cursor.read<EPropertyTagFlags>();
    static constexpr uint8_t ExtraDataMask = (uint8_t) EPropertyTagFlags::HasArrayIndex |
                                             (uint8_t) EPropertyTagFlags::HasPropertyGuid |
                                             (uint8_t) EPropertyTagFlags::HasPropertyExtensions;

    // The value of a bool is this flag, it takes no bytes in the stream.
    if (hasFlag(tag.Flags, EPropertyTagFlags::BoolTrue)) {
      if (tag.TagData != nullptr) [[likely]] {
        tag.TagData->BoolValue = true;
      }
    }
    if ((uint8_t) tag.Flags & ExtraDataMask) [[unlikely]] {
      if (hasFlag(tag.Flags, EPropertyTagFlags::HasArrayIndex)) {
        tag.ArrayIndex = cursor.read<uint32_t>();
      }
      if (hasFlag(tag.Flags, EPropertyTagFlags::HasPropertyGuid)) {
        tag.PropertyGuid = cursor.read<FGuid>();
      }
      if (hasFlag(tag.Flags, EPropertyTagFlags::HasPropertyExtensions)) {
        auto extensions = cursor.read<EPropertyTagExtension>();
        if (hasFlag(extensions, EPropertyTagExtension::OverridableInformation)) {
          cursor += 5;
        }
      }
    }

    return tag;
  }

  template<>
  FPropertyTagData *PP6::parsePropertyTagData(sc::MonotonicResource *storage, EPropertyTagType type,
                                              std::span<FPropertyTypeNameNode> typeName, const FName &name) {
    // Only these types have something in it, the others (Int, Float, Name, Object, ...) are read without it.
    switch (type) {
      case EPropertyTagType::Struct:
      case EPropertyTagType::Bool:
      case EPropertyTagType::Byte:
      case EPropertyTagType::Enum:
      case EPropertyTagType::Optional:
      case EPropertyTagType::Array:
      case EPropertyTagType::Set:
      case EPropertyTagType::Map:
        break;
      default:
        return nullptr;
    }
    auto *data = storage->make<FPropertyTagData>();
    data->Name = name;

    switch (type) {
      case EPropertyTagType::Struct: {
        if (const auto pram = getParameter(typeName, 0); !pram.empty()) {
          data->structName = pram[0].Name;
          data->StructType = static_cast<ESatisfactoryStruct>(pram[0].NameHash);
        }
        break;
      }
      case EPropertyTagType::Bool: {
        data->BoolValue = false;
        break;
      }
      case EPropertyTagType::Byte:
      case EPropertyTagType::Enum: {
        if (const auto pram = getParameter(typeName, 0); !pram.empty()) {
          data->EnumName = pram[0].Name;
        }
        break;
      }
      case EPropertyTagType::Optional:
      case EPropertyTagType::Array:
      case EPropertyTagType::Set: {
        if (const auto pram = getParameter(typeName, 0); !pram.empty()) {
          data->InnerType = static_cast<EPropertyTagType>(pram[0].NameHash);
          if (data->InnerType != EPropertyTagType::None && pram[0].innerCount != 0) {
            data->InnerTypeData = parsePropertyTagData(storage, data->InnerType, pram, pram[0].Name);
          }
        }

        break;
      }
      case EPropertyTagType::Map: {
        const auto keyType = getParameter(typeName, 0);
        const auto valType = getParameter(typeName, 1);

        if (!keyType.empty() && !valType.empty()) {
          data->InnerType = static_cast<EPropertyTagType>(keyType[0].NameHash);
          if (data->InnerType != EPropertyTagType::None && keyType[0].innerCount != 0) {
            data->InnerTypeData = parsePropertyTagData(storage, data->InnerType, keyType, keyType[0].Name);
          }
          data->ValueType = static_cast<EPropertyTagType>(valType[0].NameHash);
          if (data->ValueType != EPropertyTagType::None && valType[0].innerCount != 0) {
            data->ValueTypeData = parsePropertyTagData(storage, data->ValueType, valType, valType[0].Name);
          }
        }
        break;
      }
      default:
        break;
    }
    return data;
  }

  template<typename T>
  static inline FPropertyValue read(ByteCursor &cursor) {
    return FPropertyValue{cursor.read<T>()};
  }

  template<>
  FPropertyValue PP6::parsePropertyValue(ByteCursor &cursor, sc::MonotonicResource *storage, FPropertyTag &tag,
                                         ReadType readType, uint32_t size) {
    property_count++;
    switch (tag.PropertyType) {
      case EPropertyTagType::Struct: {
        return parseStructPropertyValue(cursor, storage, tag.TagData, readType, size);
      }
      case EPropertyTagType::Array: {
        return parseArrayPropertyValue(cursor, storage, tag.TagData, readType, size);
      }
      case EPropertyTagType::Map: {
        return parseMapProperty(cursor, storage, tag.TagData, size);
      }

      case EPropertyTagType::Bool: {
        return PP3::parseBoolPropertyValue(cursor, storage, tag.TagData, readType);
      }
      case EPropertyTagType::Byte: {
        if (tag.TagData == nullptr || tag.TagData->EnumName.isNone()) {
          return FPropertyValue{cursor.read<uint8_t>()};
        }
      }
      case EPropertyTagType::Enum: {
        return FPropertyValue{cursor.readFName()};
      }
      case EPropertyTagType::Text: {
        return PP3::parseTextPropertyValue(cursor, storage);
      }
      case EPropertyTagType::Interface:
      case EPropertyTagType::Object: {
        return PP3::parseObjectPropertyValue(cursor, storage);
      }
      case EPropertyTagType::Set: {
        return PP3::parseSetPropertyValue(cursor, storage, tag.TagData);
      }
      case EPropertyTagType::Delegate: {
        cursor += sizeof(uint32_t);
      }
      case EPropertyTagType::Name: {
        return FPropertyValue{cursor.readFName()};
      }
      case EPropertyTagType::Str: {
        return FPropertyValue{cursor.readFString()};
      }

      case EPropertyTagType::Int8: {
        return FPropertyValue{cursor.read<int8_t>()};
      };
      case EPropertyTagType::Int16: {
        return FPropertyValue{cursor.read<int16_t>()};
      }
      case EPropertyTagType::Int: {
        return FPropertyValue{cursor.read<int32_t>()};
      }
      case EPropertyTagType::Int64: {
        return FPropertyValue{cursor.read<int64_t>()};
      }
      case EPropertyTagType::UInt16: {
        return FPropertyValue{cursor.read<uint16_t>()};
      }
      case EPropertyTagType::UInt32: {
        return FPropertyValue{cursor.read<uint32_t>()};
      }
      case EPropertyTagType::UInt64: {
        return FPropertyValue{cursor.read<uint64_t>()};
      }
      case EPropertyTagType::Float: {
        return FPropertyValue{cursor.read<float>()};
      }
      case EPropertyTagType::Double: {
        return FPropertyValue{cursor.read<double>()};
      }

      case EPropertyTagType::SoftObject: {
        return FPropertyValue{SoftObj{
            cursor.readFName(),
            cursor.readFName(),
            cursor.readArray<uint8_t>(),
        }};
      }

      case EPropertyTagType::FieldPath:
      case EPropertyTagType::MulticastSparse:
      case EPropertyTagType::MulticastInline: {
        // Same as the skipped structs in parseStructPropertyValue(): in a map or a set there is no size.
        if (size == 0) [[unlikely]] {
          throw std::runtime_error("Field path or multicast delegate without a size cannot be skipped @ " +
                                   std::to_string(cursor.tell()));
        }
        cursor += size;
        break;
      }
      case EPropertyTagType::None:
        break;
      case EPropertyTagType::Unknown:
      default: {
        throw std::runtime_error("unknown property tag: " + std::string(tag.PropertyTypeName.text()) + " @ " +
                                 std::to_string(cursor.tell()));
      }
    }
    return {};
  }

  template<>
  FPropertyValue PP6::parseArrayPropertyValue(ByteCursor &cursor, sc::MonotonicResource *storage,
                                              FPropertyTagData *tagData, ReadType type, uint32_t size) {
    if (tagData == nullptr) [[unlikely]] {
      // Without the type of the elements they can neither be read nor skipped.
      throw std::runtime_error("Array without an element type @ " + std::to_string(cursor.tell()));
    }
    auto arrayType = tagData->InnerType;
    if (arrayType == EPropertyTagType::Unknown) [[unlikely]]
      return {};

    // size is 0 for an array inside a map or another array, nothing says where such an array ends.
    const bool sizeKnown  = size >= sizeof(uint32_t);
    const size_t arrayEnd = cursor.tell() + size;

    auto lenght = cursor.read<uint32_t>();
    // Every element takes at least one byte, anything above that is a broken length.
    if (lenght > cursor.remaining()) [[unlikely]] {
      throw std::runtime_error("Array length exceeds the remaining data @ " + std::to_string(cursor.tell()));
    }
    // InnerTypeData only exists for types with parameters (struct, enum, array, map, ...).
    FPropertyTag tag{.TagData = tagData->InnerTypeData};

    // Plain bytes stay where they are in the file buffer. As elements of the vector below every byte would take
    // sizeof(FPropertyValue), a 16 MiB array then needs 1.9 GB.
    // An enum stored as a name is told apart by the enum in the type name, or by taking more than one byte per
    // element where the size is known.
    const bool isEnumArray = (tagData->InnerTypeData != nullptr && !tagData->InnerTypeData->EnumName.isNone()) ||
                             (sizeKnown && (size - sizeof(uint32_t)) > lenght);
    if (arrayType == EPropertyTagType::Byte && !isEnumArray) {
      return FPropertyValue{cursor.readArray<uint8_t>(lenght)};
    }

    std::pmr::vector<FPropertyValue> vec(cursor.m_owner->storage());
    vec.reserve(lenght);

    if (arrayType == EPropertyTagType::Byte) {
      tag.PropertyType = EPropertyTagType::Enum;
      for (uint32_t i = 0; i < lenght; i++) {
        vec.emplace_back(parsePropertyValue(cursor, storage, tag, ReadType::ARRAY, 0));
      }
      return FPropertyValue{std::move(vec)};
    }

    tag.PropertyType = arrayType;
    for (uint32_t i = 0; i < lenght; i++) {
      // Only a struct takes a size, for the kinds that are skipped instead of parsed (Transform, InstancedStruct,
      // GameplayTagContainer, ...). The elements have no size of their own, so the rest of the array is split
      // evenly: exact for elements of one size, and with different sizes the last one still ends where the array
      // ends. The size of the array is always known here: UE has no nested containers, an array is never an
      // element of a map, set or another array. A struct that is a key or value of a map or an element of a set
      // gets no size at all, parseStructPropertyValue() throws if it would have to skip one.
      uint32_t elementSize = 0;
      if (arrayType == EPropertyTagType::Struct && sizeKnown && arrayEnd > cursor.tell()) {
        elementSize = static_cast<uint32_t>((arrayEnd - cursor.tell()) / (lenght - i));
      }
      vec.emplace_back(parsePropertyValue(cursor, storage, tag, ReadType::ARRAY, elementSize));
    }
    return FPropertyValue{std::move(vec)};
  }

  template<>
  PropertyHolder PP6::parseFallbackStruct(ByteCursor &cursor, sc::MonotonicResource *storage) {
    PropertyMapBuilder properties;
    while (true) {
      auto tag = parsePropertyTag(cursor, storage);
      if (tag.Name.isNone()) {
        break;
      };
      const auto end = cursor.tell() + tag.Size;
      tag.Tag = parsePropertyValue(cursor, storage, tag, ReadType::NORMAL, tag.Size);
      properties.add(PropertyHolder::propertyKey(cursor.m_owner->nameHash(tag.Name), tag.ArrayIndex), std::move(tag));
      cursor = end;
    }
    return PropertyHolder(properties.build(cursor.m_owner->storage()));
  }

  template<>
  FPropertyValue PP6::parseStructPropertyValue(ByteCursor &cursor, sc::MonotonicResource *storage,
                                               FPropertyTagData *tagData, ReadType type, uint32_t size) {
    FPropertyValue val;
    // Logger::logInfo("[PropertyHolder]", "Struct Property:{} @ {}",  tagData->structName.text(), cursor.tell());
    auto debug = cursor.tell();
    (void) debug;
    switch (tagData != nullptr ? tagData->StructType : ESatisfactoryStruct::Unknown) {
      case ESatisfactoryStruct::Guid: {
        return FPropertyValue{cursor.read<FGuid>()};
      }
      case ESatisfactoryStruct::IntPoint: {
        return FPropertyValue{cursor.read<IntPoint>()};
      }
      case ESatisfactoryStruct::Vector2f:
      case ESatisfactoryStruct::DeprecateSlateVector2D: {
        return FPropertyValue{cursor.read<FVector2F>()};
      }
      case ESatisfactoryStruct::Vector4:
      case ESatisfactoryStruct::Vector4d: {
        return FPropertyValue{cursor.read<FVector4D>()};
      }
      case ESatisfactoryStruct::Vector3f: {
        return FPropertyValue{cursor.read<FVector3F>()};
      }
      case ESatisfactoryStruct::Vector2D: {
        return FPropertyValue{cursor.read<FVector2D>()};
      }
      case ESatisfactoryStruct::IntVector: {
        return FPropertyValue{cursor.read<FIntVector>()};
      }
      case ESatisfactoryStruct::Quat: {
        return FPropertyValue{cursor.read<FQuat>()};
      }
      case ESatisfactoryStruct::Box3f: {
        return FPropertyValue{cursor.read<FBox3f>()};
      }
      case ESatisfactoryStruct::Box2f: {
        return FPropertyValue{cursor.read<FBox2f>()};
      }
      case ESatisfactoryStruct::Box: {
        return FPropertyValue{cursor.read<FBox>()};
      }
      case ESatisfactoryStruct::Vector: {
        return FPropertyValue{cursor.read<FVector>()};
      }
      case ESatisfactoryStruct::LinearColor: {
        return FPropertyValue{cursor.read<FLinearColor>()};
      }
      case ESatisfactoryStruct::Color: {
        return FPropertyValue{cursor.read<FColor>()};
      }
      case ESatisfactoryStruct::RichCurveKey: {
        return FPropertyValue{cursor.read<FRichCurveKey>()};
      }

      case ESatisfactoryStruct::Rotator: {
        return FPropertyValue{cursor.read<FRotator>()};
      }

      case ESatisfactoryStruct::IntVector2: {
        return FPropertyValue{cursor.read<FIntVector2>()};
      }
      // --- Sequencer / MovieScene (Cutscene Data) ---
      case ESatisfactoryStruct::MovieSceneTrackIdentifier:
        cursor += 4;
        break;
      case ESatisfactoryStruct::MovieSceneTrackIdentifiers:
      case ESatisfactoryStruct::MovieSceneFloatChannel:
      case ESatisfactoryStruct::MovieSceneDoubleChannel:
      case ESatisfactoryStruct::MovieSceneFrameRange:
      case ESatisfactoryStruct::MovieSceneEvaluationFieldEntityTree:
      case ESatisfactoryStruct::MovieSceneSequenceID:
      case ESatisfactoryStruct::MovieSceneEvaluationKey:
      case ESatisfactoryStruct::MovieSceneEventParameters:
      case ESatisfactoryStruct::MovieSceneSegment:
      case ESatisfactoryStruct::MovieSceneSegmentIdentifier:
      case ESatisfactoryStruct::MovieSceneTrackFieldData:
      case ESatisfactoryStruct::MovieSceneSubSequenceTree:
      case ESatisfactoryStruct::MovieSceneSequenceInstanceDataPtr:
      case ESatisfactoryStruct::MovieSceneEvalTemplatePtr:
      case ESatisfactoryStruct::MovieSceneTrackImplementationPtr:
      case ESatisfactoryStruct::MovieSceneByteChannel:
      case ESatisfactoryStruct::SectionEvaluationDataTree:

      // --- Niagara / VFX (Particle-Logic) ---
      case ESatisfactoryStruct::NiagaraUserRedirectionParameterStore:
      case ESatisfactoryStruct::NiagaraVariable:
      case ESatisfactoryStruct::NiagaraVariableBase:
      case ESatisfactoryStruct::NiagaraVariableWithOffset:
      case ESatisfactoryStruct::NiagaraDataChannelVariable:
      case ESatisfactoryStruct::NiagaraDataInterfaceGPUParamInfo:
      case ESatisfactoryStruct::NiagaraScript:
      case ESatisfactoryStruct::NiagaraScriptExecutionParameterStore:

      // --- UI / Fonts / Special-System ---
      case ESatisfactoryStruct::FontData:
      case ESatisfactoryStruct::FontCharacter:
      case ESatisfactoryStruct::LevelSequenceObjectReferenceMap:
      case ESatisfactoryStruct::SmartName:

      // Data Bloat
      case ESatisfactoryStruct::InstancedStruct:
      case ESatisfactoryStruct::Transform:
      case ESatisfactoryStruct::DateTime:
      case ESatisfactoryStruct::FrameNumber:
      case ESatisfactoryStruct::FGServerCertificate:
      case ESatisfactoryStruct::PerPlatformFrameRate:
      case ESatisfactoryStruct::ShaderValueTypeHandle:
      case ESatisfactoryStruct::GameplayTagContainer:
      case ESatisfactoryStruct::InventoryItem: {
        // As a key or value of a map or an element of a set the size is 0. Skipping nothing would read everything
        // after it from the wrong position, the file fails instead. Does not occur in 1009 or 1017.
        if (size == 0) [[unlikely]] {
          throw std::runtime_error("Struct " + std::string(tagData->structName.text()) +
                                   " without a size cannot be skipped @ " + std::to_string(cursor.tell()));
        }
        cursor += size;
        break;
      }

      case ESatisfactoryStruct::PerQualityLevelInt: {
        return FPropertyValue{FPerQualityLevelInt::get(cursor)};
        break;
      }
      case ESatisfactoryStruct::PerQualityLevelFloat: {
        return FPropertyValue{FPerQualityLevelFloat::get(cursor)};
        break;
      }
      case ESatisfactoryStruct::CompressedRichCurve: {
        return FPropertyValue{FCompressedRichCurve::get(cursor)};
      }

      case ESatisfactoryStruct::MaterialOverrideNanite: {
        return FPropertyValue{FMaterialOverrideNanite{cursor.readFName(), cursor.readBool(),
                                                      cursor.m_owner->resolveIndex(cursor.read<int32_t>())

        }};
      }
      case ESatisfactoryStruct::SkeletalMeshSamplingLODBuiltData: {
        return FPropertyValue{FWeightedRandomSampler{
            .probabilities = cursor.readArray<float>(),
            .alias = cursor.readArray<int32_t>(),
            .totlalWeight = cursor.read<float>(),
        }};
      }
      case ESatisfactoryStruct::SoftObjectPath:
      case ESatisfactoryStruct::SoftClassPath: {
        return FPropertyValue{SoftObj{cursor.readFName(), cursor.readFName(), cursor.readArray<uint8_t>()}};
      }
      case ESatisfactoryStruct::PerPlatformInt: {
        cursor.readBool();
        return FPropertyValue{cursor.read<int32_t>()};
      }
      case ESatisfactoryStruct::PerPlatformFloat: {
        cursor.readBool();
        return FPropertyValue{cursor.read<float>()};
      }

      case ESatisfactoryStruct::KeyHandleMap:
        return FPropertyValue{PropertyHolder{}};

      case ESatisfactoryStruct::UniversalObjectLocatorFragment: {
        (void) cursor.readFName(); // FragmentTypeID
      }
      default:
        //         if (tagData) {
        //           propertyLOG::info("Parsing Struct: {} as Fallback @ {}", tagData->structName.text(),
        //                           cursor.tell());
        //         }
        return FPropertyValue{parseFallbackStruct(cursor, storage)};
    }
    return val;
  }

  template<>
  FPropertyValue PP6::parseMapProperty(ByteCursor &cursor, sc::MonotonicResource *storage, FPropertyTagData *tagData,
                                       uint32_t size) {
    // InnerTypeData/ValueTypeData only exist for types with parameters (struct, enum, array, ...). Plain types
    // (name, int, str, object, ...) are read without them.
    const auto start = cursor.tell();
    FPropertyTag keyTag{.PropertyType = tagData->InnerType, .TagData = tagData->InnerTypeData};
    FPropertyTag valueTag{.PropertyType = tagData->ValueType, .TagData = tagData->ValueTypeData};

    auto keysToRemove = cursor.read<int32_t>();
    for (auto i = 0; i < keysToRemove; i++) {
      parsePropertyValue(cursor, storage, keyTag, ReadType::MAP, 0);
    }

    std::pmr::vector<std::pair<FPropertyValue, FPropertyValue>> vec(cursor.m_owner->storage());
    auto entries = cursor.read<uint32_t>();
    // Every entry takes at least two bytes, anything above that is a broken count.
    if (entries > cursor.remaining() / 2) [[unlikely]] {
      throw std::runtime_error("Map entry count exceeds the remaining data @ " + std::to_string(cursor.tell()));
    }
    vec.reserve(entries);
    for (uint32_t i = 0; i < entries; i++) {
      vec.emplace_back(parsePropertyValue(cursor, storage, keyTag, ReadType::MAP, 0),
                       parsePropertyValue(cursor, storage, valueTag, ReadType::MAP, 0));
    }

    // size is 0 for a map inside an array or another map, there is nothing to check against then.
    if (size != 0 && cursor.tell() != start + size) [[unlikely]] {
      throw std::runtime_error("Map does not end where its size says @ " + std::to_string(cursor.tell()));
    }
    return {.data = std::move(vec)};
  }
} // namespace SAA
