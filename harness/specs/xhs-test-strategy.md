# Spec: XHS Test Strategy

Date: 2026-06-15
Status: Current

## Context

`XHSPlugin` needs two different test modes:

- plugin artifact tests that exercise the built shared library through the host
  ABI;
- development unit tests that call internal process functions such as
  `xhsapi::getID`.

## Goal

Keep artifact tests and internal unit tests in separate test binaries so each
binary links against the correct library form:

- shared library target for plugin ABI and exported DLL behavior;
- static library target for internal function tests.

## Non-Goals

- Do not change XHS runtime behavior.
- Do not expose new production APIs only for tests.
- Do not make URL parser tests depend on network access.

## Behavior

`XHSPlugin_artifact_test` links against the shared `XHSPlugin` target and tests
the plugin lifecycle/export surface as a host would use it.

`XHSPlugin_unit_test` links against `XHSPlugin_static` and defines
`PLUGIN_LOCAL` while compiling tests so headers declare internal symbols with
local linkage semantics instead of DLL import semantics.

`XHSPlugin/test/CMakeLists.txt` is intentionally written as a reusable plugin
test harness. Other plugins can copy it and only change:

```cmake
set(TestTarget XHSPlugin)
```

The expected layout is:

```text
test/
|-- CMakeLists.txt
|-- plugin/
|   `-- *_test.cpp
`-- unit/
    `-- *_test.cpp
```

Files under `plugin/` become shared-library artifact tests. Files under `unit/`
become static-library unit tests. Artifact tests keep their runtime output
directory beside the plugin shared library. Unit tests use CMake's default
executable output location.

## Acceptance Criteria

- Shared-library tests can call `pluginInit()` and inspect plugin metadata.
- Static-library tests can call `xhsapi::getID()` and `xhsapi::isValidUrl()`
  directly.
- URL parser tests remain deterministic and require no network access.
- Both test binaries are registered with CTest when `ENABLE_TEST=ON`.
