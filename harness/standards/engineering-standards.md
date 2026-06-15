# Engineering Standards

Date: 2026-06-15

## Scope Control

- Keep changes scoped to the requested plugin or shared module.
- Prefer existing plugin patterns over new abstractions.
- Do not reformat unrelated files.
- Preserve user work already present in the worktree.

## C++ And ABI

- Use C++20 consistently.
- Preserve exported lifecycle functions in `TemplatePluginCall.cpp`.
- Preserve `plugin::IPlugin` method semantics and common host data contracts.
- Keep plugin singleton lifetime clear to the host.

## Layer Boundaries

- Put URL parsing, request construction, response parsing, DTOs, constants, and
  client logic in `src/<ServiceApi>/`.
- Put host-facing conversion, login adapters, downloader construction, plugin
  metadata, and resources in `src/Plugin/`.
- Keep lifecycle entry points isolated in `src/TemplatePluginCall.cpp`.

## Build And Dependencies

- Minimum CMake version is 3.15.
- Use target-based CMake APIs when practical.
- Keep dependencies in `vcpkg.json`.
- Prefer existing dependencies before adding new packages.
- Do not commit downloaded package artifacts or generated dependency output.
- Do not add `CMakeLists.txt` to documentation-only top-level directories.

## Error Handling And Security

- Invalid URLs must not crash.
- Missing optional remote fields must not crash conversion.
- Network failures should use existing client/downloader error paths.
- Do not log cookies, tokens, authorization headers, or other secrets.

## Testing

- Prefer deterministic tests for URL parsers, conversion, and error handling.
- Clearly identify tests that depend on live websites, network access, or
  credentials.
- Run the narrowest harness that can catch the expected failure mode.

## Generated Output

- Do not edit generated output under `build/` or `out/`.
- Do not treat installed runtime dependencies as source changes.
