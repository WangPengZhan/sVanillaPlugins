#pragma once

#include <BaseVideoView.h>
#include "WeiboApi/WeiboApi.h"

adapter::VideoView convertVideoView(const weiboapi::ComponentPlayPlayinfoResponse& data);
adapter::VideoView convertVideoView(const weiboapi::WeiboAjaxData& data);

adapter::BaseVideoView convertMediaInfoItem(const weiboapi::MediaInfoItem& data);