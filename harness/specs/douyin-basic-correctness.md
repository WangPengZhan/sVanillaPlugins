# DouYin Basic Correctness

Date: 2026-07-22

## Context

The DouYin plugin accepts URLs that it cannot resolve, routes series and user
requests with incorrect endpoint/parameter combinations, reports every session
as logged out, and can loop forever when a remote cursor does not advance.

## Goal

- Keep `canParseUrl()` consistent with URL types the plugin can resolve.
- Resolve direct works, user pages, mixes, and series with the matching API.
- Stop pagination on API errors, empty pages, or a cursor that does not advance.
- Derive login state from authenticated cookies and clear the session on logout.
- Reject empty detail/play URLs instead of creating invalid download tasks.
- Preserve correct downloader state and propagate FFmpeg merge failures.
- Generate `a_bogus` from the exact encoded query and body that are sent.
- Reuse `PluginCrypto` for generic SM3 and RC4 primitives; keep only the
  DouYin-specific payload and custom alphabet logic in the plugin.

## Non-goals

- Reversing a new DouYin signature implementation.
- Claiming live-network compatibility for history or collection endpoints.
- Adding live-stream or channel downloading.

## Acceptance Criteria

- Deterministic URL tests cover user pages and modal work URLs.
- Unsupported live/channel URLs are rejected.
- Series and user requests use their declared endpoint and parameter names.
- Pagination has progress and response-status guards.
- Login cookie import, logout, and downloader state do not return hard-coded
  success values.
- The plugin builds and its unit tests pass through the repository harness.
- Changing either query or body changes a deterministic `a_bogus` test vector.
