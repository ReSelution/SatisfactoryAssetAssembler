#pragma once

#include <cstddef>

#include "sc/threading/threading.hpp"

namespace SAA {
  // Groesse eines Arrays mit einem Eintrag pro Thread, indiziert mit sc::threading::thread_id(): 0 ist der Thread,
  // der init() aufgerufen hat, die Pool-Worker haben 1 bis n.
  // TODO: Greift auf sc::threading::impl zu, weil master keine oeffentliche Abfrage der Thread-Anzahl hat.
  // Durch sc::threading::thread_slots() ersetzen, sobald der ecs-Branch von StructuredChaos auf master ist.
  [[nodiscard]] inline size_t threadSlots() { return sc::threading::impl::threads.size() + 1; }
} // namespace SAA
