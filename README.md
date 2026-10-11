# SAA - Satisfactory Asset Assembler

**SAA** is an ultra-high-performance C++23 toolset engineered for parsing, decompressing, and assembling Unreal Engine 5 assets, specifically optimized for *Satisfactory*. 

---



## ⚖️ Legal Disclaimer (Oodle Compression)

**Satisfactory Asset Assembler (SAA)** does **not** ship with any proprietary Oodle binaries or copyrighted code from Epic Games / RAD Game Tools.

* **Linux Users:** You are responsible for providing your own legal copy of `liboo2corelinux64.so.9` SAA only links against this library if present in your app Dir.
* **Windows / Wine Users:** SAA utilizes **Binary Pattern Matching (AOB Scanning)** to locate and interface with the Oodle decompression logic already present within your legally owned game executables. No code is redistributed; the tool merely hooks into existing logic on your local machine.

*Oodle is a registered trademark of Epic Games Tools (formerly RAD Game Tools). This project is an independent fan-made tool and is not affiliated with, endorsed by, or sponsored by Coffee Stain Studios or Epic Games.*

## 🛠️ Build Instructions

Requires a C++23 compiler, [Meson](https://mesonbuild.com) 1.1 or newer and Ninja.
All dependencies are fetched as Meson subprojects.

```bash
git clone https://github.com/ReSelution/SatisfactoryAssetAssembler.git
cd SatisfactoryAssetAssembler
meson setup build --buildtype=release
meson compile -C build
```

Release is the default build type. Meson picks the default compiler of the system, set `CXX` before
`meson setup` to choose another one. `-Db_lto=true` enables link time optimization.

SAA is developed and tested on Linux (Fedora) with Clang. It should build on other systems as well, but that is
not tested regularly.

### Options

| Option | Default | Effect |
| --- | --- | --- |
| `enable_chaos_stats` | `true` | Compiles the StructuredChaos stats in instead of leaving them out |
| `dump_magpie` | `false` | Keeps namespace and key of every text, for `Magpie::dump()` |
| `track_properties` | `false` | Compiles the `PropertyTracker` in, it logs the properties no asset class reads. Takes a global lock on every property read |
| `native_arch` | `true` | Builds with `-march=native`, ignored when cross compiling |

## ▶️ Usage

```bash
build/src/AssetRipperTest [options] <game-path>
```

`<game-path>` is the install directory of Satisfactory. Without it the environment variable
`SAA_GAME_PATH` is used. Output (`output/`, `logs/`) is written below the current directory.
On Windows the binary is `build\src\AssetRipperTest.exe`.

> **Note:** SAA has only been tested against the Steam version of Satisfactory.

| Option | Effect |
| --- | --- |
| `-j`, `--threads <n>` | Worker threads of the pool, default: one less than the CPU has. `1` runs everything on the main thread, for debugging |
| `-h`, `--help` | Shows the help |
