// src/main.cpp
#include <filesystem>
#include <iostream>

#include "provider/games/satisfactory/SatisfactoryDataProvider.hpp"

int main() {
  std::filesystem::path gamePath = "/home/oleub/.local/share/Steam/steamapps/common/Satisfactory";

  auto start = std::chrono::high_resolution_clock::now();
  SAP::SatisfactoryDataProvider provider(gamePath);


  if (provider.load()) {
    auto end = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "SAP Lib init in: " << us.count()/1000 << "ms" << '\n';
  } else {
    std::cerr << "FEHLER: SAP-Initialisierung fehlgeschlagen. Prüfen Sie Pfade und Oodle/Mappings." << '\n';
    return 1;
  }
  // provider.test();
  provider.crawl();
  auto end = std::chrono::high_resolution_clock::now();
  auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  std::cout << "SAP Bibliothek Fertig: " << us.count() / 1000.0 << " ms\n";
  return 0;
}
