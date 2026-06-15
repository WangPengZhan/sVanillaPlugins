# Spec: Plugin Architecture

Date: 2026-06-15
Status: Current

## Context

Each implementation plugin builds as a shared library loaded by the sVanilla
host. Common contracts come from `ThirdParty/sVanillaPluginCommon`.

## Runtime ABI

Each plugin must preserve these exported lifecycle functions:

```cpp
extern "C" {
void PLUGIN_API initDir(const char* dir);
PluginHandle PLUGIN_API pluginInit();
void PLUGIN_API pluginDeinit();
void PLUGIN_API deinit();
}
```

Expected lifecycle:

1. `initDir(dir)` stores the host-provided runtime directory and initializes
   plugin-level resources such as logging.
2. `pluginInit()` creates or returns the singleton plugin instance and returns
   it as `PluginHandle`.
3. The host uses the handle as a `plugin::IPlugin`.
4. `pluginDeinit()` destroys the singleton plugin instance.
5. `deinit()` releases global registrations and final resources.

Do not rename these functions, change signatures, or change ownership semantics
unless the host ABI is intentionally migrated.

## Layering

`src/<ServiceApi>/` owns platform behavior:

- URL parsing and normalization;
- ID extraction;
- HTTP request construction;
- response parsing;
- DTOs, constants, and client helpers.

`src/Plugin/` owns host adaptation:

- `plugin::IPlugin` implementation;
- login adapter;
- conversion to host video view types;
- downloader construction;
- plugin resources and metadata.

`src/TemplatePluginCall.cpp` should contain lifecycle entry points only.

## `IPlugin` Behavior

The plugin adapter normally implements:

- `pluginMessage()`
- `websiteIcon()`
- `canParseUrl(const std::string& url)`
- `getVideoView(const std::string& url)`
- `getDownloader(const VideoInfoFull& videoInfo)`
- `loginer()`

`canParseUrl` should be cheap and predictable. URL parsing should reject
unrelated domains and malformed inputs without crashing.

`getVideoView` should validate input, resolve source records, convert them to
host view items, set `pluginId`, and return an empty view when no valid content
is available.

`getDownloader` should preserve existing ownership patterns and return a
`std::shared_ptr<download::FileDownloader>`.

## Error Handling

- Invalid URLs fail predictably.
- Missing optional response fields do not crash conversion.
- Network failures are surfaced through existing client/downloader mechanisms.
- Sensitive cookies, tokens, and authorization headers are not logged.

## Acceptance Criteria

- Plugin lifecycle ABI remains unchanged.
- Source and adapter responsibilities remain separated.
- Invalid input is handled without process crashes.
- Downloader lifetime remains clear to the host.
