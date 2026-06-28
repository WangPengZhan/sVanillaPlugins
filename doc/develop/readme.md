# Developer Documentation

This directory contains technical documentation for building, testing, and
extending `sVanillaPlugins`.

## Reading order

1. [Build and test guide](build-and-test.md)
2. [Plugin API contract](plugin-api.md)
3. [Plugin development guide](plugin-development.md)
4. [Supported URL reference](supported-urls.md)

## Key concepts

- Every implementation plugin is a C++20 shared library.
- Every plugin exports the same C ABI from `TemplatePluginCall.h`.
- The host talks to plugins through `plugin::IPlugin`.
- `getVideoView(url)` converts a website URL into selectable download items.
- `getDownloader(videoInfo)` creates the downloader for one selected item.
- Login is abstracted behind `LoginProxy`.

Use `TemplatePlugin` for scaffolding and as the repository entry point for
shared common libraries and logging. Existing implementation plugins are
references for real website behavior, but they should not introduce separate
common-library or logging setup.
