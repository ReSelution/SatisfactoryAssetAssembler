// src/main.cpp
#include <cstdint>
#include <exception>
#include <filesystem>
#include <limits>
#include <print>
#include <string>

#include <CLI/CLI.hpp>

#include "sc/ecs/registry.hpp"
#include "sc/logger/logger.hpp"
#include "sc/threading/threading.hpp"

#include "provider/games/satisfactory/SatisfactoryDataProvider.hpp"
#include "util/runtime_options.hpp"


using mainLog = sc::Logger<"SAA", "TEST">;

namespace {
  struct CommandLine {
    std::filesystem::path gamePath;
    uint32_t threads = 0;
    std::string culture;
  };

  // Every subcommand that works on the game files takes the install directory as its positional.
  void addGamePath(CLI::App &sub, CommandLine &cmd) {
    sub.add_option("game-path", cmd.gamePath, "install directory of Satisfactory")
        ->envname("SAA_GAME_PATH")
        ->required()
        ->check(CLI::ExistingDirectory);
  }
} // namespace

int main(int argc, char **argv) {
  CommandLine cmd;

  CLI::App app{"Extracts the assets of Satisfactory.", "AssetRipperTest"};
  app.footer("Output (output/, logs/) is written below the current directory.");
  // Global options are also accepted after the subcommand.
  app.fallthrough();
  app.require_subcommand(1);

  app.add_option("-j,--threads", cmd.threads,
                 "worker threads of the pool, default: one less than the CPU has.\n"
                 "1 runs everything on the main thread, for debugging.")
      ->check(CLI::Range(uint32_t{1}, std::numeric_limits<uint32_t>::max()));
  app.add_option("--culture", cmd.culture, "culture of the texts, e.g. de. Default: the native culture of the game.");

  CLI::App *crawl = app.add_subcommand("crawl", "crawl the game and export the assets");
  addGamePath(*crawl, cmd);
  CLI::App *test = app.add_subcommand("test", "parse every package of the game instead of crawling, to check the parsers");
  addGamePath(*test, cmd);

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    return app.exit(e) == 0 ? 0 : 2;
  }

  SAA::runtimeOptions().threads = cmd.threads;
  SAA::runtimeOptions().serial = cmd.threads == 1;
  SAA::runtimeOptions().culture = cmd.culture;

  try {
    sc::ecs::Registry reg;

    auto t = mainLog::time(spdlog::level::err, "Finished in {}");

    // Auch seriell gibt es den Pool: einzelne Stellen geben weiter Arbeit dorthin ab.
    if (cmd.threads > 1) {
      sc::threading::init(cmd.threads);
    } else {
      sc::threading::init();
    }
    SAA::SatisfactoryDataProvider provider(cmd.gamePath);
    SAA::SatisfactoryDataProvider::setECS(&reg);
    provider.load();

    if (test->parsed()) {
      provider.test();
    } else {
      provider.crawl();
    }
  } catch (const std::exception &e) {
    std::println(stderr, "Error: {}", e.what());
    return 1;
  }
  return 0;
}
