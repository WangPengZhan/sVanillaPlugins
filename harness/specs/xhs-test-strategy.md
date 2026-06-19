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

`XHSPlugin/test/CMakeLists.txt` is self-contained and does not depend on a
top-level repository `cmake/` helper directory. It derives the plugin root and
repository root from `CMAKE_CURRENT_LIST_DIR` so the plugin can be configured as
an independent CMake source directory or added as a subdirectory.

The expected layout is:

```text
test/
|-- CMakeLists.txt
|-- plugin/
|   `-- *_test.cpp
`-- unit/
    `-- unit_test.cpp
```

Files under `plugin/` become shared-library artifact tests. The single
`unit/unit_test.cpp` file becomes the static-library unit test. Artifact tests
keep their runtime output directory beside the plugin shared library. Unit tests
use CMake's default executable output location.

## Acceptance Criteria

- Shared-library tests can call `pluginInit()` and inspect plugin metadata.
- Static-library tests can call `xhsapi::getID()` and `xhsapi::isValidUrl()`
  directly.
- URL parser tests remain deterministic and require no network access.
- Both test binaries are registered with CTest when `ENABLE_TEST=ON`.
