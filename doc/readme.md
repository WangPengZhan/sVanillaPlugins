# sVanillaPlugins Documentation

This directory contains user and developer documentation for the
`sVanillaPlugins` repository.

## Documents

- [User manual](manual.md): supported sites, supported URL types, login notes,
  and the normal download workflow.
- [Developer build guide](develop/build-and-test.md): local setup, CMake
  configuration, build, install, tests, and formatting.
- [Developer documentation index](develop/readme.md): recommended reading order
  for development documents.
- [Plugin development guide](develop/plugin-development.md): how to create or
  update a plugin by following the existing plugin structure.
- [Plugin API contract](develop/plugin-api.md): exported C ABI, `IPlugin`,
  video view metadata, download configuration, downloader lifecycle, and login
  proxy behavior.
- [Supported URL reference](develop/supported-urls.md): URL patterns handled by
  each implementation plugin.

## Project scope

`sVanillaPlugins` provides C++20 shared-library plugins for sVanilla. The
plugins parse supported website URLs, expose downloadable media items as
`BaseVideoView` records, and create `FileDownloader` instances that download the
selected item to a configured directory.

Current implementation plugins cover:

- BiliBili
- Dedao
- DouYin
- HLS streams
- NetEase Cloud Music
- Weibo
- Xiaohongshu
- YouTube

`TemplatePlugin` is a starter implementation for new plugins.
