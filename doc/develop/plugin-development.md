# Plugin Development Guide

Use `TemplatePlugin` as the starting point for new plugins. It is also the
repository entry point for shared common libraries and logging support. Existing
implementation plugins follow the same basic shape and should consume those
shared targets instead of introducing their own common-library or logging setup.

## Recommended structure

```text
NewSitePlugin/
|-- CMakeLists.txt
|-- src/
|   |-- NewSiteApi/
|   |   |-- NewSiteApi.h
|   |   |-- NewSiteClient.h
|   |   |-- NewSiteClient.cpp
|   |   |-- NewSiteUrl.h
|   |   `-- NewSiteUrl.cpp
|   |-- Plugin/
|   |   |-- NewSitePlugin.h
|   |   |-- NewSitePlugin.cpp
|   |   |-- NewSitePluginMessage.h
|   |   |-- NewSiteDownloader.h
|   |   |-- NewSiteDownloader.cpp
|   |   |-- NewSiteLogin.h
|   |   `-- NewSiteLogin.cpp
|   `-- TemplatePluginCall.cpp
`-- test/
    |-- CMakeLists.txt
    |-- plugin/
    |   `-- test_plugin.cpp
    `-- unit/
        `-- unit_test.cpp
```

## Add the plugin to the root build

The root `CMakeLists.txt` automatically scans top-level directories and calls
`add_subdirectory` for any directory that contains a `CMakeLists.txt`.

To add a plugin:

1. Place the plugin directory at the repository root.
2. Add a plugin-local `CMakeLists.txt`.
3. Make sure the plugin target name matches the plugin project name.
4. Add artifact tests under `test/plugin/` and unit tests under `test/unit/`.
5. Build from the repository root.

## Implement metadata

Create a plugin message header with stable metadata:

```cpp
namespace newsiteplugin
{
constexpr int pluginID = 9;
constexpr char name[] = "NewSite";
constexpr char version[] = "1.0.0";
constexpr char description[] = "this is a plugin for sVanilla to download NewSite video.";
constexpr char domain[] = "https://example.com";
}
```

Use a unique `pluginID`; do not reuse existing IDs.

## Implement URL parsing

Keep URL parsing isolated in the API layer, usually in `NewSiteUrl.h/.cpp`.

Recommended functions:

```cpp
bool isValidUrl(const std::string& url);
IDInfo getID(const std::string& url);
```

The plugin should call `isValidUrl` from `canParseUrl` and should avoid network
work there unless the URL cannot be validated locally.

## Implement `getVideoView`

`getVideoView` should:

1. Validate and normalize the input URL.
2. Resolve the source item or collection.
3. Convert source API records into `adapter::BaseVideoView`.
4. Set `pluginId` on every returned item.
5. Fill `Identifier`, `IdType`, `Title`, `Publisher`, `Cover`, `Duration`,
   `PublishDate`, and `fileExtension` when available.

Return an empty vector when the URL is invalid or the source has no downloadable
items.

## Implement `getDownloader`

`getDownloader` should:

1. Copy `VideoInfoFull`, `DownloadConfig`, and `BaseVideoView` before storing or
   modifying them.
2. Resolve final media URLs as late as possible so login cookies and quality
   options are current.
3. Set output directory and file name from `DownloadConfig` and
   `VideoInfoFull::fileName()`.
4. Return a `download::FileDownloader`.

If a website returns separate video and audio streams, use the existing common
download helpers and FFmpeg helper patterns used by the BiliBili, YouTube, and
HLS plugins.

## Implement login

If the website supports account login, implement either:

- `AbstractLoginApi` for QR-code or API polling login;
- `AbstractLoginWeb` for browser or cookie-based login.

The host interacts with the implementation through `LoginProxy`, so keep login
state, cookies, user info, and logout behavior behind that proxy.

## Resources

Website icons and QR-code UI resources are currently embedded as generated
`std::vector<uint8_t>` arrays. Keep generated resource headers scoped to the
plugin that uses them.

## Testing

Add tests for:

- plugin initialization;
- metadata;
- URL parser behavior;
- `getVideoView` conversion with representative URLs;
- downloader creation.

Prefer parser tests that do not require network access. Treat real website tests
as integration tests and keep them easy to disable or diagnose.

When the plugin follows the current repository pattern, keep host/ABI tests in
`test/plugin/` and internal deterministic tests in `test/unit/`.
