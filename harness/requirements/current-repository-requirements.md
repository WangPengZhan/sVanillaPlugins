# Current Repository Requirements

Date: 2026-06-15

## Product Requirements

- The repository provides shared-library plugins that the sVanilla host can
  load through the common plugin ABI.
- Each plugin must expose metadata, URL recognition, video view resolution,
  downloader creation, and login access through `plugin::IPlugin`.
- `TemplatePlugin` must remain a usable reference for creating new plugins.
- Existing supported URL behavior should remain stable unless a task explicitly
  changes it.

## Compatibility Requirements

- Preserve C++20 compatibility.
- Preserve Windows/MSVC compatibility, including existing parallel compile
  settings in plugin CMake files.
- Keep Linux and macOS portability unless a plugin is explicitly
  platform-specific.
- Preserve the exported lifecycle ABI in every `TemplatePluginCall.cpp`.

## Build Requirements

- The root build must continue using the repository-local vcpkg toolchain.
- New plugins should be top-level directories with plugin-local
  `CMakeLists.txt` files.
- Documentation and AI harness directories must remain build-neutral and must
  not contain top-level `CMakeLists.txt` files.
- Generated output under `build/` and `out/` is not source.

## Testing Requirements

- Use `python scripts/harness.py` for verification.
- Enable tests with `-EnableTest` when tests are relevant to the change.
- Prefer deterministic tests for URL parsing, conversion, and error handling.
- Document network, credential, or live-platform test limitations when they
  affect verification.

## Documentation Requirements

- Non-trivial behavior changes need a short SDD note before implementation.
- Final AI handoffs must include changed files, verification commands, results,
  and remaining risk.
- Shared AI context should live in `harness/` when it is useful to multiple
  agents.
