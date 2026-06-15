# Plugin API Contract

This document describes the ABI and C++ interfaces used by sVanilla plugins.

## Exported C ABI

Each plugin exports the functions declared in `TemplatePluginCall.h`:

```cpp
extern "C" {
void PLUGIN_API initDir(const char* dir);
PluginHandle PLUGIN_API pluginInit();
void PLUGIN_API pluginDeinit();
void PLUGIN_API deinit();
}
```

Expected lifecycle:

1. `initDir(dir)`: provide the plugin working directory. Plugins use this path
   for login QR-code files, covers, temporary files, and other runtime data.
2. `pluginInit()`: create or return the plugin singleton and return it as a
   `PluginHandle`.
3. Use the returned handle as a `plugin::IPlugin`.
4. `pluginDeinit()`: destroy the plugin instance.
5. `deinit()`: final cleanup hook.

## IPlugin

All implementation plugins derive from `plugin::IPlugin`:

```cpp
class IPlugin
{
public:
    virtual const PluginMessage& pluginMessage() const = 0;
    virtual const std::vector<uint8_t>& websiteIcon() = 0;
    virtual bool canParseUrl(const std::string& url) = 0;
    virtual adapter::VideoView getVideoView(const std::string& url) = 0;
    virtual std::shared_ptr<download::FileDownloader> getDownloader(const VideoInfoFull& videoInfo) = 0;
    virtual LoginProxy loginer() = 0;
};
```

## Plugin metadata

`PluginMessage` identifies a plugin:

```cpp
struct PluginMessage
{
    int pluginId;
    std::string name;
    std::string version;
    std::string description;
    std::string domain;
};
```

Current plugin IDs:

| ID | Plugin |
| --- | --- |
| `0` | Template |
| `1` | BiliBili |
| `2` | Youtube |
| `3` | HLSStream |
| `4` | WeiBo |
| `5` | NeteaseCloudMusic |
| `6` | Dedao |
| `7` | XHSPlugin |
| `8` | DouYinPlugin |

## Video views

`getVideoView(url)` returns `adapter::VideoView`, a list of
`adapter::BaseVideoView` items. Each item is one selectable download candidate.

Important fields:

| Field | Meaning |
| --- | --- |
| `Identifier` | Primary source identifier or URL used later by `getDownloader`. |
| `IdType` | Plugin-specific identifier type. |
| `ParentId` | Parent collection, playlist, channel, course, or series ID. |
| `ParentIdType` | Plugin-specific type for `ParentId`. |
| `Title` | Display title and default file-name source. |
| `Publisher` | Uploader, author, artist, or publisher. |
| `Cover` | Cover image URL or generated local cover path. |
| `Duration` | Human-readable duration when available. |
| `Description` | Source description when available. |
| `PublishDate` | Source publish date when available. |
| `PlayListTitle` | Parent list title when available. |
| `fileExtension` | Output extension. Defaults to `.mp4`. |
| `fileType` | Output type, such as video, audio, image, PDF, markdown, or text. |
| `Option1` to `Option3` | Plugin-specific extra data. |
| `pluginId` | Owning plugin ID. |

## Download configuration

The host creates `VideoInfoFull` from a selected `BaseVideoView` and a
`DownloadConfig`.

```cpp
struct DownloadConfig
{
    std::string downloadDir;
    VideQuality videoQuality = VideQuality::Best;
    std::string nameRule = "$title$";
};
```

`VideoInfoFull::fileName()` applies the `nameRule`, sanitizes the result, and
appends the selected item's extension.

## Downloader lifecycle

`getDownloader(videoInfo)` returns a `download::FileDownloader`.

Downloader operations:

- `start()`
- `stop()`
- `pause()`
- `resume()`
- `downloadStatus()`
- `finish()`

Downloader status values:

- `Waiting`
- `Ready`
- `Downloading`
- `Pause`
- `Paused`
- `Resumed`
- `Stopped`
- `Finished`
- `Error`

The host can read progress through `info()`, which contains total bytes,
completed bytes, speed, and stage text.

## Login proxy

`loginer()` returns a `LoginProxy`. The proxy wraps either an API login or a web
login implementation.

Common operations:

- `supportsLogin()`
- `isLoggedIn()`
- `cookies()`
- `setCookies(cookies)`
- `refreshCookies(cookies)`
- `logout()`
- `getUserInfo(dir)`
- `history()`

API-based logins can additionally provide QR-code scan content, scan status, and
login resources such as background, loading, wait-confirmation, complete, and
refresh images.
