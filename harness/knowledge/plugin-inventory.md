# Plugin Inventory

Date: 2026-06-15

## Current Plugins

| Plugin | Purpose |
| --- | --- |
| `BiliBiliPlugin` | BiliBili video, bangumi, cheese, media list, and user-space content. |
| `DedaoPlugin` | Dedao live, ebook, course, and course article content. |
| `DouYinPlugin` | DouYin user, video, note, slides, collection, live, and shared URL content. |
| `HLSPlugin` | Generic HLS playlists whose fetched content starts with `#EXTM3U`. |
| `NeteaseCloudMusicPlugin` | NetEase Cloud Music songs, albums, playlists, top lists, and MVs. |
| `TemplatePlugin` | Reference plugin structure for new implementations. |
| `WeiboPlugin` | Weibo mobile, desktop, TV, and video-show content. |
| `XHSPlugin` | Xiaohongshu note/explore and user profile content requiring `xsec_token`. |
| `YoutubePlugin` | YouTube videos, shorts, playlists, channels, handles, Music, and Kids URLs. |

## Plugin IDs

The current documented plugin IDs are:

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

Before adding a plugin, verify the ID in source and documentation and choose a
new unique value.

## URL Surface

Canonical details live in `doc/develop/supported-urls.md`. Use that document
when editing parser behavior. Keep parser tests focused on:

- valid representative URLs;
- malformed URLs;
- unrelated domains;
- short-link or share-link expansion assumptions;
- required query fields such as `xsec_token` where applicable.
