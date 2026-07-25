# Cross-Plugin Test Parity

## Context

The Bilibili plugin test module now separates deterministic unit coverage from
live plugin business flows, starts the shared Aria runtime through a global test
environment, classifies JSON cases by URL type, exercises downloader creation
and completion, and provides a standalone expected-view generator.

Other implementation plugins currently have uneven coverage, with most plugin
tests limited to metadata and synthetic URL recognition.

## Goal

Apply the same testing principles to Dedao, Douyin, HLS, Netease Cloud Music,
Weibo, XHS, and YouTube while respecting each service's authentication and URL
model.

## Ownership Boundary

- Tests for `sVanillaPluginCommon` crypto, networking, FFmpeg, downloader
  infrastructure, and shared interfaces belong only to the Bilibili unit-test
  target.
- Every other plugin tests its own URL analysis, request/signing helpers,
  response conversion, parser/cipher logic, downloader boundaries, and exported
  lifecycle where applicable.
- Plugin-specific coverage must not be replaced by duplicated common-library
  tests.

## Required Behavior

- Unit tests remain deterministic and cover URL parsing plus important signing,
  conversion, request-shaping, and error-boundary logic exposed by each plugin.
- Plugin tests initialize and deinitialize the exported lifecycle safely.
- Tests that can create downloaders register a global Aria environment, reuse an
  already-running RPC service, and stop only a process they started.
- Live business cases declare `isSmokeTest`, `linkType`, `description`, `url`,
  download configuration, expected views, downloader expectations, download
  expectations, and timeout.
- URL types are exposed as individually selectable ordinary GTest cases; smoke
  cases have a separate selectable test.
- Credential-dependent services may retain deterministic artifact tests when a
  stable anonymous business flow cannot be established; this limitation must be
  explicit in the test source or case data.
- Standalone expected-view generation is added only where anonymous live view
  resolution is supported and must not be compiled into the normal GTest target.

## Acceptance

- Every affected plugin test and unit-test target compiles.
- Normal tests do not require opt-in environment variables.
- Existing user changes and plugin lifecycle ABI remain unchanged.
- Live downloads are not required for compile-only verification; remaining
  credential and network risks are reported.
