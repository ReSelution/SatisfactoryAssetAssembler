// src/main.cpp
#include <charconv>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <optional>
#include <print>
#include <string_view>
#include <system_error>

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
    bool help = false;
  };

  void printUsage(std::FILE *out, const char *program) {
    std::println(out,
                 "Usage: {} [options] <game-path>\n\n  <game-path>        install directory of Satisfactory, or set "
                 "SAA_GAME_PATH\n\nOptions:\n  -j, --threads <n>  worker threads of the pool, default: one less than "
                 "the CPU has.\n                     1 runs everything on the main thread, for debugging.\n  -h, "
                 "--help         show this help\n\nOutput (output/, logs/) is written below the current directory.",
                 program);
  }

  // Leer, wenn die Argumente nicht passen. Die Meldung dazu ist dann schon ausgegeben.
  std::optional<CommandLine> parseCommandLine(int argc, char **argv) {
    CommandLine cmd;
    for (int i = 1; i < argc; ++i) {
      const std::string_view arg = argv[i];
      if (arg == "-h" || arg == "--help") {
        cmd.help = true;
        return cmd;
      }
      if (arg == "-j" || arg == "--threads") {
        if (i + 1 >= argc) {
          std::fprintf(stderr, "Missing value for %s\n", argv[i]);
          return std::nullopt;
        }
        const std::string_view value = argv[++i];
        const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), cmd.threads);
        if (error != std::errc() || end != value.data() + value.size() || cmd.threads == 0) {
          std::fprintf(stderr, "Not a thread count: %s\n", argv[i]);
          return std::nullopt;
        }
        continue;
      }
      if (arg.starts_with('-')) {
        std::fprintf(stderr, "Unknown option: %s\n", argv[i]);
        return std::nullopt;
      }
      if (!cmd.gamePath.empty()) {
        std::fprintf(stderr, "More than one game path: %s\n", argv[i]);
        return std::nullopt;
      }
      cmd.gamePath = arg;
    }

    if (cmd.gamePath.empty()) {
      if (const char *env = std::getenv("SAA_GAME_PATH"); env != nullptr && *env != '\0') {
        cmd.gamePath = env;
      }
    }
    return cmd;
  }
} // namespace

int main(int argc, char **argv) {
  const char *program = argc > 0 ? argv[0] : "AssetRipperTest";

  const std::optional<CommandLine> cmd = parseCommandLine(argc, argv);
  if (!cmd) {
    printUsage(stderr, program);
    return 2;
  }
  if (cmd->help) {
    printUsage(stdout, program);
    return 0;
  }
  if (cmd->gamePath.empty()) {
    printUsage(stderr, program);
    return 2;
  }
  if (!std::filesystem::is_directory(cmd->gamePath)) {
    std::println(stderr, "Not a directory: {}", cmd->gamePath.string());
    return 2;
  }

  SAA::runtimeOptions().threads = cmd->threads;
  SAA::runtimeOptions().serial = cmd->threads == 1;

  try {
    sc::ecs::Registry reg;

    auto t = mainLog::time(spdlog::level::err, "Finished in {}");

    // Auch seriell gibt es den Pool: einzelne Stellen geben weiter Arbeit dorthin ab.
    if (cmd->threads > 1) {
      sc::threading::init(cmd->threads);
    } else {
      sc::threading::init();
    }
    SAA::SatisfactoryDataProvider provider(cmd->gamePath);
    SAA::SatisfactoryDataProvider::setECS(&reg);
    provider.load();

    // provider.test();
    provider.crawl();
  } catch (const std::exception &e) {
    std::println(stderr, "Error: {}", e.what());
    return 1;
  }
  return 0;
}
