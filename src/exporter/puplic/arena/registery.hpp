//
// Created by oleub on 22.02.26.
//

#pragma once
#include <concepts>
#include <cstring>
#include <memory>
#include <memory_resource>
#include <span>

#include "puplic/structs/meta_export.hpp"

struct MetaExport;

namespace SAP {
  template<typename T>
  struct is_span : std::false_type {
  };

  template<typename T, std::size_t Extent>
  struct is_span<std::span<T, Extent> > : std::true_type {
  };

  template<typename T>
  inline constexpr bool is_span_v = is_span<std::decay_t<T> >::value;

  template<typename Tpram>
  static constexpr size_t get_size_safe(const Tpram &arg) {
    if constexpr (is_span_v<Tpram>) {
      return arg.size_bytes();
    } else {
      return 0;
    }
  }


  class Registry {
  public:

    static void init(size_t bufferSize);

    template<typename T, typename... Args>
      requires std::derived_from<T, MetaExport>
    static T *write(Args &&... args);

    template<typename T>
    static std::span<T *> get_all() {
      auto &vec = registry[static_cast<size_t>(T::Kind)];
      return std::span<T *>(reinterpret_cast<T **>(vec.data()), vec.size());
    }

    static void reset();

  private:
    static void *internal_allocate(size_t size);

    static inline std::unique_ptr<uint8_t[]> backingBuffer;
    static inline std::unique_ptr<std::pmr::monotonic_buffer_resource> pool;
    static inline std::array<std::vector<void *>, static_cast<size_t>(ExportType::COUNT)> registry;
    static inline std::atomic_flag m_allocLock = ATOMIC_FLAG_INIT;
    static inline std::atomic_flag m_registryLock = ATOMIC_FLAG_INIT;
    static inline bool initialized = false;
  };

  template<typename T, typename... Args> requires std::derived_from<T, MetaExport>
  T *Registry::write(Args &&... args) {
    size_t spansByteSize = (0 + ... + get_size_safe(args));
    size_t totalSize = sizeof(T) + spansByteSize;
    void *ptr = internal_allocate(totalSize);
    std::byte *currentDataPtr = static_cast<std::byte *>(ptr) + sizeof(T);

    auto transformed = std::make_tuple([&]<typename Tpram>(Tpram &&arg) {
      if constexpr (is_span_v<Tpram>) {
        if (!arg.empty()) {
          std::memcpy(currentDataPtr, arg.data(), arg.size_bytes());
        }
        using S = std::decay_t<Tpram>;
        auto newSpan = S(reinterpret_cast<typename S::value_type *>(currentDataPtr), arg.size());
        currentDataPtr += arg.size_bytes();
        return newSpan;
      } else {
        return std::forward<Tpram>(arg);
      }
    }(std::forward<Args>(args))...);

    T* result = nullptr;
    std::apply([&]<typename... T0>(T0 &&... finalArgs) {
     result = new(ptr) T({},std::forward<T0>(finalArgs)...);
    }, std::move(transformed));

    while (m_registryLock.test_and_set(std::memory_order_acquire)) {
    }
    registry[static_cast<size_t>(T::Kind)].push_back(ptr);
    m_registryLock.clear();
    return result;
  }
} // SAP
