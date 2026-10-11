# SAA - Satisfactory Asset Assembler

**SAA** is an ultra-high-performance C++20 toolset engineered for parsing, decompressing, and assembling Unreal Engine 5 assets, specifically optimized for *Satisfactory*. 

---



## ⚖️ Legal Disclaimer (Oodle Compression)

**Satisfactory Asset Assembler (SAA)** does **not** ship with any proprietary Oodle binaries or copyrighted code from Epic Games / RAD Game Tools.

* **Linux Users:** You are responsible for providing your own legal copy of `liboo2corelinux64.so.9` SAA only links against this library if present in your app Dir.
* **Windows / Wine Users:** SAA utilizes **Binary Pattern Matching (AOB Scanning)** to locate and interface with the Oodle decompression logic already present within your legally owned game executables. No code is redistributed; the tool merely hooks into existing logic on your local machine.

*Oodle is a registered trademark of Epic Games Tools (formerly RAD Game Tools). This project is an independent fan-made tool and is not affiliated with, endorsed by, or sponsored by Coffee Stain Studios or Epic Games.*

## 🛠️ Build Instructions (Linux / Fedora)

Requires a C++23 compiler, [Meson](https://mesonbuild.com) 1.1 or newer and Ninja.
All dependencies are fetched as Meson subprojects.

```bash
git clone https://github.com/ReSelution/SatisfactoryAssetAssembler.git
cd SatisfactoryAssetAssembler
meson setup build/release --native-file cross/linux-clang.ini --buildtype=release -Db_lto=true -Db_ndebug=true
meson compile -C build/release
```

`cross/linux-clang.ini` selects Clang with mold and ccache. `-Db_ndebug=true` matters: without it the
crawler, the provider and the localizations run on a single thread. Other configurations:

```bash
# Optimized with debug symbols, for profiling
meson setup build/relwithdebinfo --native-file cross/linux-clang.ini -Doptimization=3 -Ddebug=true -Db_ndebug=true

# AddressSanitizer + UndefinedBehaviorSanitizer
meson setup build/debug --native-file cross/linux-clang.ini -Db_sanitize=address,undefined

# Windows, cross compiled with clang-cl (SDK via scripts/setup-win-sdk.sh)
meson setup build/win64 --cross-file cross/win64-clang-cl.ini --buildtype=release
```

### Options

| Option | Default | Effect |
| --- | --- | --- |
| `enable_chaos_stats` | `true` | Compiles the StructuredChaos stats in instead of leaving them out |
| `dump_magpie` | `false` | Keeps namespace and key of every text, for `Magpie::dump()` |
| `native_arch` | `true` | Builds with `-march=native`, ignored when cross compiling |
