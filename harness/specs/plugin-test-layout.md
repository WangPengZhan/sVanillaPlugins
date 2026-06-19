# Spec: Plugin Test Layout

Date: 2026-06-15
Status: Current

## Context

Plugin tests need two modes:

- artifact tests that exercise the built shared library through the exported
  plugin ABI;
- unit tests that exercise internal functions through a static library target.

## Goal

Keep every plugin test directory self-contained so each plugin can be configured
as an independent CMake source directory or included as a subdirectory of the
repository build. Test CMake logic must live in the plugin's own
`test/CMakeLists.txt`; the repository root must not provide a top-level
`cmake/` helper directory for plugin tests.

## Layout

```text
test/
|-- CMakeLists.txt
|-- plugin/
|   `-- *_test.cpp
`-- unit/
    `-- unit_test.cpp
```

Files under `plugin/` are linked against the shared plugin target and keep the
test executable output beside the plugin shared library.

Files under `unit/` are linked against `<PluginName>_static`, define
`PLUGIN_LOCAL`, and use the default CMake executable output location. Each
plugin must define `<PluginName>_static` in its own top-level `CMakeLists.txt`
before adding the `test/` subdirectory. The plugin-local `test/CMakeLists.txt`
consumes that target and must not create plugin static libraries itself.

## Requirements

- Every implementation plugin should have both `test/plugin/*.cpp` and `test/unit/*.cpp`.
- Site URL parser tests should cover at least 10 supported URL examples per
  plugin when the plugin owns a URL parser.
- Plugin artifact tests should exercise the shared-library ABI and deterministic
  URL recognition only.
- Unit tests should prefer deterministic parsing, conversion, and helper
  behavior.
- Live network or credential-dependent tests should be kept in artifact tests
  only when they are intentionally integration tests.
- HLS `canParseUrl` currently downloads remote playlist content, so default HLS
  artifact tests verify metadata only and parser coverage lives in unit tests.
