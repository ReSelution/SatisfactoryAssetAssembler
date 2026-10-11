// src/main.cpp
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>

#include "provider/games/satisfactory/SatisfactoryDataProvider.hpp"
#include "sc/ecs/registry.hpp"
#include "sc/logger/logger.hpp"
#include "sc/threading/threading.hpp"


using mainLog = sc::Logger<"SAA", "TEST">;

// The install directory of the game: the first argument, or SAA_GAME_PATH if there is none.
static std::filesystem::path gamePathFrom(int argc, char **argv) {
  if (argc > 1) {
    return argv[1];
  }
  if (const char *env = std::getenv("SAA_GAME_PATH"); env != nullptr && *env != '\0') {
    return env;
  }
  return {};
}

int main(int argc, char **argv) {
  const std::filesystem::path gamePath = gamePathFrom(argc, argv);
  if (gamePath.empty()) {
    std::fprintf(stderr,
                 "Usage: %s <game-path>\n"
                 "  <game-path>  install directory of Satisfactory, or set SAA_GAME_PATH\n"
                 "Output (output/, logs/) is written below the current directory.\n",
                 argc > 0 ? argv[0] : "AssetRipperTest");
    return 2;
  }
  if (!std::filesystem::is_directory(gamePath)) {
    std::fprintf(stderr, "Not a directory: %s\n", gamePath.string().c_str());
    return 2;
  }

  try {
    sc::ecs::Registry reg;

    auto t = mainLog::time(spdlog::level::err, "Finished in {}");

    sc::threading::init();
    SAA::SatisfactoryDataProvider provider(gamePath);
    SAA::SatisfactoryDataProvider::setECS(&reg);
    provider.load();

    // provider.test();
    provider.crawl();
  } catch (const std::exception &e) {
    std::fprintf(stderr, "Error: %s\n", e.what());
    return 1;
  }
  return 0;
}
