#pragma once

#include <BaseVideoView.h>
#include "DouYinApi/DouYinApi.h"

std::string getCover(const douyinapi::Video& video);
std::string getPlayUrl(const douyinapi::Video& video);
douyinapi::FollowingUser getUserInfo(const douyinapi::FollowingResponse& following);

adapter::BaseVideoView convertAwemeDetail(const douyinapi::AwemeDetail& detail);

adapter::VideoView convertSeriesDetail(const douyinapi::SeriesDetail& detail);
