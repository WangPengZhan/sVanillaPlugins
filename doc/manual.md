# sVanillaPlugins User Manual

This manual explains what the plugins download and how the host application is
expected to use them.

## Overview

Each plugin accepts a URL from a supported website, checks whether it can parse
the URL, resolves one or more downloadable items, and returns a downloader for
the selected item.

The common flow is:

1. Load the plugin shared library.
2. Initialize the plugin directory with `initDir`.
3. Create the plugin instance with `pluginInit`.
4. Call `canParseUrl(url)`.
5. Call `getVideoView(url)` to list downloadable items.
6. Create a `VideoInfoFull` with a `DownloadConfig` and one selected
   `BaseVideoView`.
7. Call `getDownloader(videoInfo)`.
8. Start, pause, resume, stop, or inspect the downloader through the downloader
   interface.
9. Release the plugin with `pluginDeinit` and `deinit`.

## Supported plugins

| Plugin | What it downloads | Login support |
| --- | --- | --- |
| BiliBili | Videos, bangumi, cheese courses, favorites, and user video lists. | QR login and cookie refresh. |
| Dedao | Live pages, courses, articles, and ebook-related resources exposed by the plugin. | QR login and cookie refresh. |
| DouYin | Video, note, slides, collections, series, user links, live-related links, and modal links. | QR login and cookie refresh. |
| HLSStream | Direct HLS `.m3u8` playlists and master playlists. | No site account is required by the plugin itself. |
| NeteaseCloudMusic | Songs, albums, playlists, top lists, and MVs. | QR login and cookie refresh. |
| WeiBo | Status/detail URLs and Weibo video show URLs. | QR login and cookie refresh. |
| XHSPlugin | Xiaohongshu notes and user profile links that contain `xsec_token`. | QR login and cookie refresh. |
| Youtube | Videos, Shorts, embedded videos, playlists, channels, handles, user pages, and custom channels. | Cookie-based web login support. |

## Download configuration

The host passes download settings through `DownloadConfig`:

| Field | Description |
| --- | --- |
| `downloadDir` | Output directory for downloaded files. |
| `videoQuality` | Requested quality. `Best` lets the plugin choose the best available option. |
| `nameRule` | File name template. Defaults to `$title$`. |

Supported file name template tags are:

| Tag | Value |
| --- | --- |
| `$id$` | Item identifier. |
| `$title$` | Item title. |
| `$publisher$` | Publisher or uploader. |
| `$publishdate$` | Publish date from the source site when available. |
| `$date$` | Current local date. |
| `$time$` | Current local time. |
| `$datetime$` | Current local date and time. |

The generated file name is sanitized and the selected item's file extension is
appended automatically.

## Login

Most website plugins expose a `LoginProxy` through `loginer()`. Depending on the
plugin, the login implementation can provide QR-code resources, cookies, user
information, logout, and cookie refresh.

Login is optional for public content, but it can be required when:

- the site requires an account for the selected media;
- the media is region, age, membership, or permission restricted;
- the site rate limits anonymous requests;
- higher quality streams are only available after authentication.

The host should store cookies securely and refresh them with
`refreshCookies(cookies)` when supported.

## HLS downloads

The HLS plugin handles direct playlists rather than a specific website account.
It downloads the playlist, parses media streams, selects default media streams,
downloads segments, and uses FFmpeg helper functionality to produce the final
media file. For master playlists, it picks the highest-bandwidth video stream
and includes one audio and one subtitle or closed-caption stream when available.

The HLS plugin also accepts `Option1` on the selected `BaseVideoView` as an
optional download thread count. Valid values are `1` through `20`; otherwise the
default is `5`.

## Practical limits

The plugins depend on public or internal website endpoints. A supported URL
pattern means the plugin has parser logic for that URL shape; actual downloads
can still fail if the website changes its API, blocks the request, requires
login, removes the media, or returns unavailable streams.

Always use the plugins in accordance with the source website terms and the
permissions you have for the media.
