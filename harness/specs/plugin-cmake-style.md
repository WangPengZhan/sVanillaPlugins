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
