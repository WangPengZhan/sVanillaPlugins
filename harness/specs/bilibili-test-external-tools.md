# BiliBili Test External Tools

Date: 2026-07-04

## Context

BiliBili plugin business-flow tests exercise real download and media merge paths
that depend on `aria2c` and `ffmpeg`. The repository carries platform-specific
tool directories under `tools/`.

## Goal

Make each BiliBili test executable output directory contain `aria/` and
`ffmpeg/` directories copied from the repository `tools/<platform>/` tree so
tests can run real tools without requiring system installs. Start the local
`aria2c` JSON-RPC service from the GoogleTest process before BiliBili tests run
so the downloader can talk to `http://localhost:6800/jsonrpc`. The executable
is resolved from the test executable directory's `aria/` child directory.

## Non-Goals

- Do not change plugin runtime logic.
- Do not change downloader or FFmpeg helper source behavior.
- Do not introduce system package installation requirements.
- Do not require Qt in the plugin test harness.

## Acceptance Criteria

- BiliBili plugin and unit test targets copy `tools/<platform>/aria` and
  `tools/<platform>/ffmpeg` after build when the directories exist.
- The BiliBili plugin and unit GoogleTest binaries register a global test
  environment that starts `aria2c` with the same RPC port and token used by
  `AriaClient`.
- The GoogleTest environment uses the repository's TinyProcessLib process
  helper instead of hand-rolled platform process code.
- The GoogleTest environment shuts down the `aria2c` process it started after
  tests exit.
- Windows, Linux, and macOS platform folders are selected by CMake platform.
- Existing test target runtime dependency behavior remains intact.
