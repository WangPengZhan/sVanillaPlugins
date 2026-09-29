# Spec: Plugin CMake Style

Date: 2026-06-19
Status: Current

## Context

Plugin top-level `CMakeLists.txt` files should use a consistent structure so
shared build behavior is easy to compare and maintain.

## Goal

Align implementation plugin CMake files with the `BiliBiliPlugin` layout while
preserving plugin-specific dependencies and helper targets.

## Requirements

- Each plugin keeps CMake 3.15, C++20, `UNICODE`, and MSVC parallel compile
  options.
- `TemplatePlugin` is both the reference plugin template and the repository
  entry point for introducing shared common libraries and logging support.
- Other repository locations must not call `add_subdirectory` or equivalent
  setup for `ThirdParty/sVanillaPluginCommon`, `spdlog`, or the shared logging
  stack. They should consume the targets made available through the template
  entry point.
- Shared dependencies are expressed through `DYNAMIC_LIBS`,
  `STATIC_LIBS`, `INCLUDE_DIRECTORIES_PRIVATE`, and target-based commands.
- Shared libraries used at runtime, including `spdlog` and `FFmpeg`, are copied
  beside plugin artifacts and installed consistently when linked.
- Static test targets are named `<PluginName>_static` and are created before
  adding the plugin-local `test` subdirectory. They link shared common
  dependencies normally, without `--whole-archive`, because unit-test
  executables do not need plugin export retention and must not force duplicate
  third-party archive objects into the link.
- Plugin-specific additions such as `HLSDownloader` and `LibXml2` remain local
  to the plugins that need them.
- Static helper targets that are consumed by another plugin, such as
  `HLSDownloader`, package only their own object files and do not propagate
  `--whole-archive` dependency lists. The consuming plugin remains responsible
  for linking the shared common libraries once.

## Harness Plan

Run
`python scripts/harness.py --action configure --enable-test --build-dir out/harness`
first because the change is CMake-only, then broaden to build or validate if
configure exposes no environment blocker.

## Addendum 2026-09-29: Source Globs Use `CONFIGURE_DEPENDS`

### Context

Every plugin builds its source list with a directory glob:

```cmake
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")
```

A glob is evaluated only while CMake configures. After generation, adding or
removing a file under `src/` does not change the file list used by the build,
because the incremental path never re-evaluates it. The failure mode is silent
at configure time and shows up later as a missing symbol, a source file that was
never compiled, or a stale object still being linked. Recovering requires a
manual re-configure, which is easy to forget.

The plugin `test/CMakeLists.txt` files already pass `CONFIGURE_DEPENDS` for
every test source glob, so plugin sources were the last inconsistent place.

### Behavior

- Each top-level plugin `CMakeLists.txt` declares its source glob as
  `file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.h")`.
- The change is limited to the glob keyword. No source list, target, include
  directory, link dependency, or export is modified.
- Covered plugins: `BiliBiliPlugin`, `DedaoPlugin`, `DouYinPlugin`,
  `HLSPlugin` (both the plugin target glob and the `HLSDownloader` helper glob),
  `NeteaseCloudMusicPlugin`, `WeiboPlugin`, `XHSPlugin`, `YoutubePlugin`.
- `TemplatePlugin` follows the same rule so the reference structure stops
  regenerating plugins with an unguarded glob.
- `CONFIGURE_DEPENDS` requires CMake 3.12, which is satisfied by the plugin
  `cmake_minimum_required(VERSION 3.15)`.

### Acceptance Criteria

- `out/harness/CMakeFiles/VerifyGlobs.cmake` contains a glob check for
  `<Plugin>/src/*.cpp` and `<Plugin>/src/*.h` for each covered plugin, plus
  `HLSPlugin/src/HLSApi/*.cpp` and `HLSPlugin/src/Plugin/*.cpp`.
- Every covered target still configures and links: the plugin shared library
  and `<Plugin>_static` for each plugin, and `HLSDownloader`.
- The offline test gate `XHSPlugin_unit_test` still passes.
- The generated patches are ASCII-only and pass `git apply --check --reverse`.

### Harness Plan

A plain incremental build is enough: editing a `CMakeLists.txt` marks
`ZERO_CHECK` out of date, so the next `cmake --build` re-runs CMake and
registers the glob checks. Then build the covered targets and run the offline
unit gate. Verified with:

```bash
env -u http_proxy -u https_proxy cmake --build out/harness --config Debug \
  --target BiliBiliPlugin BiliBiliPlugin_static DedaoPlugin DedaoPlugin_static \
           DouYinPlugin DouYinPlugin_static HLSPlugin HLSDownloader \
           NeteaseCloudMusicPlugin NeteaseCloudMusicPlugin_static \
           WeiboPlugin WeiboPlugin_static XHSPlugin XHSPlugin_static \
           XHSPlugin_unit_test
env -u http_proxy -u https_proxy ctest --test-dir out/harness \
  --build-config Debug -R XHSPlugin_unit_test
```

### Remaining Risk

- `CONFIGURE_DEPENDS` makes the generator check the glob at build time. With the
  Visual Studio generator this costs one directory stat pass per build; the
  observed effect was a normal incremental build with no added configure.
- `TemplatePlugin` is a git submodule. Its one-line change cannot be expressed
  in a root-relative patch, so it ships as a separate submodule-relative patch
  and must be committed in the `TemplatePlugin` repository.
