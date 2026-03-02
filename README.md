# SAA - Satisfactory Asset Assembler

**SAA** is an ultra-high-performance C++20 toolset engineered for parsing, decompressing, and assembling Unreal Engine 5 assets, specifically optimized for *Satisfactory*. 

---

## 🛠️ Build Instructions (Linux / Fedora)

## ⚖️ Legal Disclaimer (Oodle Compression)

**Satisfactory Asset Assembler (SAA)** does **not** ship with any proprietary Oodle binaries or copyrighted code from Epic Games / RAD Game Tools.

* **Linux Users:** You are responsible for providing your own legal copy of `liboo2corelinux64.so.9` SAA only links against this library if present in your app Dir.
* **Windows / Wine Users:** SAA utilizes **Binary Pattern Matching (AOB Scanning)** to locate and interface with the Oodle decompression logic already present within your legally owned game executables. No code is redistributed; the tool merely hooks into existing logic on your local machine.

*Oodle is a registered trademark of Epic Games Tools (formerly RAD Game Tools). This project is an independent fan-made tool and is not affiliated with, endorsed by, or sponsored by Coffee Stain Studios or Epic Games.*

```bash

git clone --recursive [https://github.com/ReSelution/SatisfactoryAssetAssembler.git](https://github.com/ReSelution/SatisfactoryAssetAssembler.git)
cd SatisfactoryAssetAssembler
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
