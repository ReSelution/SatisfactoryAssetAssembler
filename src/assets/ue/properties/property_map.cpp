#include "f_property_tag.hpp"

#include <memory>
#include <utility>
#include <vector>

#include "sc/memory/monotonic_resource.hpp"

namespace SAA {
  namespace {
    struct Pending {
      uint64_t key;
      FProperty property;
    };

    thread_local std::vector<Pending> pendingProperties;
  } // namespace

  PropertyMapBuilder::PropertyMapBuilder() : m_start(pendingProperties.size()) {}

  PropertyMapBuilder::~PropertyMapBuilder() { pendingProperties.resize(m_start); }

  void PropertyMapBuilder::add(uint64_t key, FPropertyTag &&tag) {
    pendingProperties.emplace_back(
        key, FProperty{.Name = tag.Name.text(), .TypeName = tag.PropertyTypeName.text(), .Tag = std::move(tag.Tag)});
  }

  PropertyMap *PropertyMapBuilder::build(sc::MonotonicResource *storage) {
    auto &pending      = pendingProperties;
    const size_t count = pending.size() - m_start;

    // Neither the map nor its values get a destructor call, see OwnsOnlyResourceMemory<FPropertyValue>.
    auto *map = new (storage->allocate(sizeof(PropertyMap), alignof(PropertyMap))) PropertyMap();
    if (count != 0) {
      auto *keys   = static_cast<uint64_t *>(storage->allocate(count * sizeof(uint64_t), alignof(uint64_t)));
      auto *values = static_cast<FProperty *>(storage->allocate(count * sizeof(FProperty), alignof(FProperty)));
      for (size_t i = 0; i < count; ++i) {
        keys[i] = pending[m_start + i].key;
        std::construct_at(values + i, std::move(pending[m_start + i].property));
      }
      map->m_keys   = keys;
      map->m_values = values;
      map->m_size   = static_cast<uint32_t>(count);
    }
    pending.resize(m_start);
    return map;
  }
} // namespace SAA
