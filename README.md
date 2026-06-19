# sVanillaPlugins

`sVanillaPlugins` contains the official template plugin and a set of implementation
plugins for the sVanilla plugin system. Each plugin is built as a C++ shared
library and uses the common plugin interfaces and helper libraries from
`sVanillaPluginCommon`.

## Included plugins

| Directory | Purpose |
| --- | --- |
| `TemplatePlugin` | Minimal plugin template for creating new sVanilla plugins. |
| `BiliBiliPlugin` | BiliBili media plugin implementation. |
| `DedaoPlugin` | Dedao plugin implementation. |
| `DouYinPlugin` | DouYin plugin implementation. |
| `HLSPlugin` | HLS playlist and media download plugin implementation. |
| `NeteaseCloudMusicPlugin` | NetEase Cloud Music plugin implementation. |
| `WeiboPlugin` | Weibo plugin implementation. |
| `XHSPlugin` | Xiaohongshu plugin implementation. |
| `YoutubePlugin` | YouTube plugin implementation. |

The root `CMakeLists.txt` automatically adds every top-level directory that
contains its own `CMakeLists.txt`, so new plugin directories can be integrated by
following the same structure.

## Repository layout

```text
.
|-- BiliBiliPlugin/
|-- DedaoPlugin/
|-- DouYinPlugin/
|-- HLSPlugin/
|-- NeteaseCloudMusicPlugin/
|-- TemplatePlugin/
|-- WeiboPlugin/
|-- XHSPlugin/
|-- YoutubePlugin/
|-- ThirdParty/
|   |-- sVanillaPluginCommon/
|   `-- vcpkg/
|-- scripts/
|-- CMakeLists.txt
`-- vcpkg.json
```

Most plugin directories follow this convention:

```text
<PluginName>/
|-- src/
|   |-- <ServiceApi>/
|   `-- Plugin/
|-- test/
|   |-- plugin/
|   `-- unit/
`-- CMakeLists.txt
```

## Requirements

- CMake 3.15 or newer
- A C++20 compiler
- Git with submodule support
- Platform build tools:
  - Windows: Visual Studio 2022 or compatible MSVC toolchain
  - Linux: GCC or Clang, plus standard development packages
  - macOS: Apple Clang and Xcode command line tools

Dependencies are resolved through the bundled `ThirdParty/vcpkg` submodule and
the root `vcpkg.json` manifest. Current manifest dependencies include `curl`,
`nlohmann-json`, `openssl`, `gtest`, and `libxml2`.

## Clone

Clone with submodules, or initialize them after cloning:

```bash
git clone --recursive <repository-url>
cd sVanillaPlugins
```

If the repository was cloned without `--recursive`, run:

```bash
git submodule sync --recursive
git submodule update --init --recursive
```

## Build

Configure and build from the repository root:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Install build outputs to a local install directory:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=installed
cmake --build build --config Release --target install
```

On Windows with Visual Studio generators, keep `--config Release` on build and
install commands because the generator is multi-config.

## Tests

Plugin tests are disabled by default. Configure with `ENABLE_TEST=ON` to include
test targets:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_TEST=ON
cmake --build build --config Release
ctest --test-dir build --build-config Release
```

## Development workflow

Use `TemplatePlugin` as the starting point for a new plugin:

1. Copy or fork the template plugin structure.
2. Rename the CMake project and exported plugin classes.
3. Implement service-specific API, login, download, and message handling code
   under `src/`.
4. Add focused artifact tests under `test/plugin/` and unit tests under
   `test/unit/`, then enable them with `ENABLE_TEST=ON`.
5. Keep source formatting consistent with the plugin-local `.clang-format`
   rules.

The helper script below can format plugin source files:

```bash
python scripts/clang_format_all.py
```

## Continuous integration

The repository includes GitHub Actions workflows for:

- Windows builds with MSVC
- Linux builds with GCC
- macOS builds with Clang
- clang-format checks for plugin source files

CI builds initialize all submodules before configuring CMake.

## Notes

- Plugin libraries define `PLUGIN_DLL_EXPORTS` and are built as shared
  libraries.
- The common sVanilla plugin libraries are provided by
  `ThirdParty/sVanillaPluginCommon`.
- `VCPKG_APPLOCAL_DEPS` is enabled so runtime dependencies can be copied next to
  built plugin binaries where supported.

## License

This repository is licensed under the terms in [LICENSE](LICENSE).
