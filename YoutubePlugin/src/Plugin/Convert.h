#pragma once

#include <BaseVideoView.h>
#include "YoutubeApi/YoutubeApi.h"

adapter::VideoView convertVideoView(const youtubeapi::MainResponse& data);
