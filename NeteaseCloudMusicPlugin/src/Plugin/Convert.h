#pragma once
#include <BaseVideoView.h>

#include "NeteaseCloudMusicApi/NeteaseCloudMusicApi.h"

enum class ContentType
{
    SONG,
    MV,
};

adapter::VideoView convertVideoView(const netease::SongDetails& data);
adapter::VideoView convertVideoView(const netease::MVResponse& data);
