# Project Overview

Date: 2026-06-15

## Repository Type

This repository is a CMake-driven C++20 collection of shared-library plugins for
the sVanilla host. Each implementation plugin is a top-level directory with its
own `CMakeLists.txt`.

Current build plugins:

- `BiliBiliPlugin`
- `DedaoPlugin`
- `DouYinPlugin`
- `HLSPlugin`
- `NeteaseCloudMusicPlugin`
- `TemplatePlugin`
- `WeiboPlugin`
- `XHSPlugin`
- `YoutubePlugin`

Every listed plugin currently has a `test/` directory and a plugin-local
`CMakeLists.txt`.

## Build Shape

The root `CMakeLists.txt`:

- sets the repository-local vcpkg toolchain at
  `ThirdParty/vcpkg/scripts/buildsystems/vcpkg.cmake`;
- enables `VCPKG_APPLOCAL_DEPS`;
- creates the aggregate `sVanillaPlugins` target from `main.cpp`;
- scans top-level child directories;
- calls `add_subdirectory` for any child that contains `CMakeLists.txt`;
- adds discovered plugin targets as dependencies of `sVanillaPlugins`.

Because of this scan rule, top-level documentation or harness folders must not
contain `CMakeLists.txt`.

## Dependencies

Dependencies are declared in `vcpkg.json`:

- `curl >= 8.0.1`
- `nlohmann-json >= 3.11.2`
- `openssl`
- `gtest`
- `libxml2 >= 2.14.5#1`

Common plugin interfaces and helper targets come from
`ThirdParty/sVanillaPluginCommon`, including `PluginCommon`, `PluginDownload`,
`NetWork`, `AriaClient`, `Util`, and `FFmpeg`.

## Source Layout

Most plugins follow this shape:

```text
<PluginName>/
|-- CMakeLists.txt
|-- src/
|   |-- TemplatePluginCall.cpp
|   |-- <ServiceApi>/
|   `-- Plugin/
`-- test/
```

`TemplatePlugin` is the reference structure for new plugins.

## Canonical Local Docs

- `doc/develop/build-and-test.md`: human build and test guide.
- `doc/develop/plugin-development.md`: plugin implementation guide.
- `doc/develop/plugin-api.md`: ABI and interface contract.
- `doc/develop/supported-urls.md`: current URL support reference.
- `harness/requirements/current-repository-requirements.md`: shared
  requirements.
- `harness/specs/plugin-architecture.md`: shared ABI and layering spec.
- `harness/workflow/verification-matrix.md`: shared harness command selection.
