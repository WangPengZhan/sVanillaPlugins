# Supported URL Reference

This document summarizes URL shapes handled by the implementation plugins. The
patterns are derived from the plugin URL parser code.

## BiliBiliPlugin

Supported URL types:

- AV video: `https://www.bilibili.com/video/av123456`
- BV video: `https://www.bilibili.com/video/BVxxxxxxxxxx`
- Short BiliBili host variants using `b23.tv`
- Bangumi season: `https://www.bilibili.com/bangumi/play/ss123456`
- Bangumi episode: `https://www.bilibili.com/bangumi/play/ep123456`
- Bangumi media: `https://www.bilibili.com/bangumi/media/md123456`
- Cheese season: `https://www.bilibili.com/cheese/play/ss123456`
- Cheese episode: `https://www.bilibili.com/cheese/play/ep123456`
- Favorite or media list: `https://www.bilibili.com/medialist/detail/ml123456`
- User space: `https://space.bilibili.com/123456`

## DedaoPlugin

Supported URL types:

- Live detail: `https://www.dedao.cn/live/detail?id=<id>`
- Ebook reader: `https://www.dedao.cn/ebook/reader?id=<id>`
- Course detail: `https://www.dedao.cn/course/detail?id=<id>`
- Course article: `https://www.dedao.cn/course/article?id=<id>`

## DouYinPlugin

Supported URL types:

- User page: `https://www.douyin.com/user/<id>`
- Shared user page: `https://www.iesdouyin.com/share/user/<id>?...`
- Video, note, or slides detail:
  `https://www.douyin.com/video/<19-digit-id>`
- Shared video, note, or slides detail:
  `https://www.iesdouyin.com/share/video/<19-digit-id>/...`
- Search or discover URLs with `modal_id=<19-digit-id>`
- Collection: `https://www.douyin.com/collection/<19-digit-id>`
- Shared mix detail:
  `https://www.iesdouyin.com/share/mix/detail/<19-digit-id>/...`
- Shared series detail:
  `https://www.iesdouyin.com/share/series/detail/<19-digit-id>/...`
- Live room: `https://live.douyin.com/<room-id>`
- Follow live URL with `webRid=<room-id>`
- Webcast reflow URLs under `https://webcast.amemv.com/douyin/webcast/reflow/`
- Channel URLs with `modal_id=<19-digit-id>`

## HLSPlugin

The HLS plugin supports URLs that return a valid HLS playlist whose content
starts with `#EXTM3U`.

Examples:

- Media playlist: `https://example.com/path/playlist.m3u8`
- Master playlist: `https://example.com/path/master.m3u8`

The plugin verifies the URL by downloading and trimming the playlist content.

## NeteaseCloudMusicPlugin

Supported URL types:

- Song: `https://music.163.com/song?id=123456`
- Mobile song URL: `https://y.music.163.com/song?id=123456`
- Album: `https://music.163.com/#/album?id=123456`
- Artist: `https://music.163.com/#/artist?id=123456`
- Playlist: `https://music.163.com/#/playlist?id=123456`
- Top list: `https://music.163.com/#/discover/toplist?id=123456`
- MV: `https://music.163.com/#/mv?id=123456`

The current plugin resolves songs, albums, playlists, top lists, and MVs into
downloadable views. Artist URL parsing exists, but the current `getVideoView`
switch does not resolve artist pages into views.

## WeiboPlugin

Supported URL types:

- Mobile status: `https://m.weibo.cn/status/<id>`
- Mobile detail: `https://m.weibo.cn/detail/<id>`
- User status URL: `https://weibo.com/<uid>/<id>`
- Video show by `fid`: `https://video.weibo.com/show?fid=<fid>`
- TV show: `https://weibo.com/tv/show/<fid>`
- H5 video show: `https://h5.video.weibo.com/show/<fid>`

## XHSPlugin

Supported URL types:

- Note, discovery item, or explore URL:
  `https://www.xiaohongshu.com/explore/<note-id>?xsec_token=<token>`
- User profile URL:
  `https://www.xiaohongshu.com/user/profile/<user-id>?xsec_token=<token>`

The current parser requires `xsec_token` in the URL.

## YoutubePlugin

Supported URL types:

- Watch page: `https://www.youtube.com/watch?v=<video-id>`
- Short URL: `https://youtu.be/<video-id>`
- Embedded video: `https://www.youtube.com/embed/<video-id>`
- Shorts: `https://www.youtube.com/shorts/<video-id>`
- Playlist: `https://www.youtube.com/playlist?list=<playlist-id>`
- Channel ID: `https://www.youtube.com/channel/<channel-id>`
- Handle: `https://www.youtube.com/@<handle>`
- User page: `https://www.youtube.com/user/<user-name>`
- Custom channel: `https://www.youtube.com/c/<custom-name>`
- YouTube Music watch URLs:
  `https://music.youtube.com/watch?v=<video-id>`
- YouTube Kids watch URLs:
  `https://www.youtubekids.com/watch?v=<video-id>`

Video IDs must be 11 characters and may contain letters, numbers, `_`, and `-`.
