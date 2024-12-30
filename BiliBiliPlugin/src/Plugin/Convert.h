#pragma once

#include <BaseVideoView.h>
#include "BiliApi/BiliApi.h"

adapter::VideoView convertVideoView(const biliapi::VideoView& data);
adapter::VideoView convertVideoView(const biliapi::History& data);

bool checkSeason(const biliapi::VideoView& data);
bool checkPages(const biliapi::VideoView& data);

adapter::BaseVideoView convertEpisodes(const biliapi::UgcEpisode& data);
adapter::BaseVideoView convertPages(const biliapi::VideoPage& data);
adapter::BaseVideoView convertSingleVideo(const biliapi::VideoView& data);
adapter::BaseVideoView convertHistory(const biliapi::HistoryInfo& data);
