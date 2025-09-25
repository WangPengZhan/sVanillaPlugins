#pragma once

#include <BaseVideoView.h>
#include "YoutubeApi/YoutubeApi.h"

adapter::VideoView convertVideoView(const youtubeapi::MainResponse& data);
adapter::VideoView convertVideoView(const youtubeapi::PlayLisTabContents& data);
adapter::BaseVideoView convertVideoInfo(const youtubeapi::PlaylistVideoRenderer& data);
