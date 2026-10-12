#pragma once

#include <cstdint>
#include <string>

namespace SAA {
  // Einstellungen fuer einen Lauf. main() setzt sie aus der Kommandozeile, bevor irgendetwas geladen wird.
  struct RuntimeOptions {
    // Anzahl der Worker im Thread-Pool. 0 nimmt den Standard von sc::threading::init().
    uint32_t threads = 0;
    // Provider, Lokalisierung und Crawler arbeiten auf dem aufrufenden Thread statt im Pool. Zum Debuggen gedacht:
    // derselbe Build laesst sich so seriell und parallel laufen lassen.
    bool serial = false;
    // Culture the texts are loaded in, "de" for example. Empty loads every localization target in its native
    // culture.
    std::string culture;
  };

  [[nodiscard]] inline RuntimeOptions &runtimeOptions() {
    static RuntimeOptions options;
    return options;
  }
} // namespace SAA
