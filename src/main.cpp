// src/main.cpp
#include <filesystem>
#include <iostream>

#include "provider/games/satisfactory/SatisfactoryDataProvider.hpp"

int main() {

  std::filesystem::path gamePath = "/home/oleub/.local/share/Steam/steamapps/common/Satisfactory";

  // Testen der Initialisierung der Bibliothek
  SAP::SatisfactoryDataProvider provider(gamePath);

  if (provider.load()) {

    std::cout << "SAP Bibliothek erfolgreich initialisiert!" << '\n';
  } else {
    std::cerr << "FEHLER: SAP-Initialisierung fehlgeschlagen. Prüfen Sie Pfade und Oodle/Mappings." << '\n';
    return 1;
  }
  provider.test();

  return 0;
}