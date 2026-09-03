# Draenor-Core

Warlords of Draenor **6.2.4.21742** server core (C++), based on TrinityCore 6.04 TDB / 6.2 (Firestorm leak).

License: [GPL-2.0](LICENSE.txt).

## What you build

| Target | Output |
| --- | --- |
| `worldserver` | Game server |
| `bnetserver` | Battle.net / login |
| map / vmap / mmap tools | Only if `-DTOOLS=1` |

On Windows the binaries land in `build/bin/<Config>/` (for example `build/bin/RelWithDebInfo/`). Config `.dist` files are copied next to them.

SQL scripts are in `sql/`.

## Prerequisites

Install these **outside** the repo. Everything else (zlib, g3dlite, protobuf, Recast, fmt, …) comes from `dep/`.

| | Windows | Linux |
| --- | --- | --- |
| CMake | ≥ 3.20.4 | ≥ 3.20.4 |
| Compiler | Visual Studio 2019 / 2022 / 2026, **x64** | GCC 7+ or Clang (C++11) |
| Git | any current release | any current release |
| Boost | ≥ 1.78, MSVC-matching libs (`BOOST_ROOT`) | ≥ 1.74 |
| OpenSSL | 1.1.x or 3.x Win64 | `libssl-dev` |
| MySQL | 8.x Server or Connector (`libmysql.lib` + headers) | `libmysqlclient-dev` |

x64 only. Do not generate the build **inside** the source tree: CMake will abort.

## Build (Windows)

1. Install VS with the C++ workload, CMake, Boost, OpenSSL-Win64, MySQL.
2. Point CMake at Boost (and OpenSSL / MySQL if they are not found automatically).

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 ^
  -DBOOST_ROOT=C:/local/boost_1_78_0 ^
  -DOPENSSL_ROOT_DIR="C:/Program Files/OpenSSL-Win64" ^
  -DMYSQL_INCLUDE_DIR="C:/Program Files/MySQL/MySQL Server 8.0/include" ^
  -DMYSQL_LIBRARY="C:/Program Files/MySQL/MySQL Server 8.0/lib/libmysql.lib" ^
  -DWITH_WARNINGS=0
```

Use the generator that matches your VS (`cmake --help`). VS 2026 is `"Visual Studio 18 2026"`. Multi-config generators ignore `CMAKE_BUILD_TYPE`; pick the config when compiling:

```bat
cmake --build build --config RelWithDebInfo --target worldserver bnetserver --parallel
```

`Release` is smaller; `RelWithDebInfo` is the usual day-to-day build.

## Build (Linux)

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DWITH_WARNINGS=0
cmake --build . --parallel
# optional: cmake --install .
```

Default install prefix is `/usr/local` (configs under `etc/firestorm` in that case). Override with `-DCMAKE_INSTALL_PREFIX=...`.

## Useful CMake options

| Option | Default | Meaning |
| --- | --- | --- |
| `SERVERS` | ON | `worldserver` + `bnetserver` |
| `SCRIPTS` | ON | Script library |
| `TOOLS` | OFF | Client data extractors (`-DTOOLS=1`) |
| `CROSS` | OFF | Cross-realm core |
| `USE_COREPCH` / `USE_SCRIPTPCH` | ON | Precompiled headers |
| `WITH_WARNINGS` | ON | Extra compiler warnings |
| `WITHOUT_GIT` | OFF | Skip git revision check |

After a CMake or toolchain change, delete `build/` and configure again. Do not mix generators in the same folder.

## After a successful build

Copy `libmysql` / `libssl` / `libcrypto` DLLs next to the Windows executables if they are not already on `PATH`. Edit `worldserver.conf` and `bnetserver.conf` from the `.dist` files. Apply SQL from `sql/`. Extract maps/vmaps/mmaps with the tools if `DataDir` does not already have them (client **6.2.4.21742**).

## Known issues

- Pathfinding / movement generators need smooth-path updates.
- Areatriggers are outdated vs Legion-era cores.
- Scene system is only partial.
- Pet Battles are incomplete.
- Phasing still uses `phasemask`, not PhaseID / PhaseGroup.
- Low-level zones are playable; unpopular ones are not fully blizzlike.
- Some NPCs have bad coordinates or duplicates; Tanaan and Hellfire Citadel NPCs are the main gaps.
- Draenor content is not blizzlike overall. Garrison intro and the legendary questline were reworked. **Hellfire Citadel is not implemented.**
- LFG and vehicles are unfinished TrinityCore cherry-picks.
