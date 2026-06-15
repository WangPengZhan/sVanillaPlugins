# Build and Test Guide

## Requirements

- CMake 3.15 or newer
- C++20 compiler
- Git with submodule support
- Platform build tools:
  - Windows: Visual Studio 2022 or compatible MSVC toolchain
  - Linux: GCC or Clang
  - macOS: Apple Clang and Xcode command line tools

Dependencies are resolved with the bundled `ThirdParty/vcpkg` submodule and the
root `vcpkg.json` manifest.

## Initialize submodules

```bash
git submodule sync --recursive
git submodule update --init --recursive
```

Required submodules include:

- `ThirdParty/vcpkg`
- `ThirdParty/sVanillaPluginCommon`
- `TemplatePlugin`

## Configure

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

For multi-config generators such as Visual Studio, `CMAKE_BUILD_TYPE` is less
important than the later `--config Release` build argument.

## Build

```bash
cmake --build build --config Release
```

On Windows CI the target is `all_build`; on Unix-like CI the target is `all`.
For local development, the default build target is normally enough.

## Install

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=installed
cmake --build build --config Release --target install
```

Plugin CMake files install the plugin shared library and runtime dependencies
such as `spdlog` or `FFmpeg` when the plugin declares them.

## Enable tests

Tests are disabled by default.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_TEST=ON
cmake --build build --config Release
ctest --test-dir build --build-config Release
```

Some tests use real website URLs. Treat those tests as integration tests: they
can fail when network access is unavailable, the website response changes, or
authentication is required.

## Format code

Run the repository formatter helper from the repository root:

```bash
python scripts/clang_format_all.py
```

The helper formats source files in directories whose names end with `Plugin`
and skips `build` and `ThirdParty`.
