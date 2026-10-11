#pragma once

#include <cstddef>
#include <memory_resource>
#include <new>

#if defined(__linux__)
#include <sys/mman.h>
#endif

namespace SAA {
  // Upstream fuer grosse, langlebige Bloecke (Reader, Vfs): holt sie direkt vom Betriebssystem und meldet sie fuer
  // Transparent Huge Pages an. Der erste Zugriff auf 2 MiB kostet dann einen Page Fault statt 512.
  // Kleine Anfragen und alles ausserhalb von Linux gehen an new/delete.
  //
  // Gemessen: die Ladephase des Providers faellt damit von ca. 12,5 ms auf ca. 10 ms.
  //
  // Setzt voraus, dass mimalloc THP nicht fuer den ganzen Prozess sperrt. StructuredChaos baut mimalloc deshalb mit
  // MI_NO_THP statt mit mi_allow_thp=off: mimalloc fragt dann selbst keine Huge Pages an, madvise() von hier greift
  // aber. Steht der THP-Modus des Systems auf "always" statt "madvise", bekommt auch der Speicher von mimalloc Huge
  // Pages und der Crawl wird wieder langsamer (ca. 65 ms statt 51 ms).
  class HugePageResource final : public std::pmr::memory_resource {
    static constexpr size_t HUGE_PAGE = size_t{2} * 1024 * 1024;

  public:
    static HugePageResource *get() {
      static HugePageResource instance;
      return &instance;
    }

  private:
    static size_t roundUp(size_t bytes) { return (bytes + HUGE_PAGE - 1) & ~(HUGE_PAGE - 1); }

    void *do_allocate(size_t bytes, size_t alignment) override {
#if defined(__linux__)
      if (bytes >= HUGE_PAGE && alignment <= HUGE_PAGE) {
        // Um eine Huge Page groesser angefordert, damit der Anfang auf 2 MiB ausgerichtet werden kann.
        const size_t size   = roundUp(bytes);
        const size_t mapped = size + HUGE_PAGE;
        void *raw           = mmap(nullptr, mapped, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (raw == MAP_FAILED) {
          throw std::bad_alloc();
        }
        auto *start   = static_cast<std::byte *>(raw);
        auto *aligned = reinterpret_cast<std::byte *>(roundUp(reinterpret_cast<size_t>(start)));
        if (aligned != start) {
          munmap(start, static_cast<size_t>(aligned - start));
        }
        if (std::byte *end = aligned + size; end != start + mapped) {
          munmap(end, static_cast<size_t>(start + mapped - end));
        }
        (void) madvise(aligned, size, MADV_HUGEPAGE);
        return aligned;
      }
#endif
      return std::pmr::new_delete_resource()->allocate(bytes, alignment);
    }

    void do_deallocate(void *ptr, size_t bytes, size_t alignment) override {
#if defined(__linux__)
      if (bytes >= HUGE_PAGE && alignment <= HUGE_PAGE) {
        munmap(ptr, roundUp(bytes));
        return;
      }
#endif
      std::pmr::new_delete_resource()->deallocate(ptr, bytes, alignment);
    }

    [[nodiscard]] bool do_is_equal(const memory_resource &other) const noexcept override { return this == &other; }
  };
} // namespace SAA
