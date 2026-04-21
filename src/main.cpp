// src/main.cpp
#include <filesystem>
#include <iostream>

#include "provider/games/satisfactory/SatisfactoryDataProvider.hpp"
#include "threading/chaos_threading.hpp"
#include "ecs/chaos_registry.hpp"


LOG_ALIAS(mainLog, "SAA", "TEST")

void setWorkingDirectory(const char* argv0) {
  namespace fs = std::filesystem;
  fs::path exePath = std::filesystem::canonical(argv0);
  fs::path projectRoot = exePath.parent_path().parent_path();
  fs::current_path(projectRoot);

}

int main(int argc, char** argv) {

  SC::ChaosRegistry reg;
  if (argc > 0) {
    setWorkingDirectory(argv[0]);
  }
  if (std::filesystem::current_path().filename() != "SatisfactoryAssetAssembler") {
    throw std::runtime_error("Wrong working Directory");
  }


  auto t = mainLog::time(spdlog::level::err, "Finished in {}");

  SC::ChaosThreading::init();
  std::filesystem::path gamePath =
      "/home/oleub/.var/app/com.valvesoftware.Steam/.local/share/Steam/steamapps/common/Satisfactory/";
  //gamePath = "/home/oleub/dev/data/Satisfactory_1009/";
  SAA::SatisfactoryDataProvider provider(gamePath);
  SAA::SatisfactoryDataProvider::setECS(&reg);
  provider.load();

  // provider.test();
  provider.crawl();
  return 0;
}
