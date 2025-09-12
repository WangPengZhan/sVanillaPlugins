#pragma once

#include <BaseVideoView.h>
#include "DedaoApi/DedaoApi.h"

adapter::VideoView convertVideoView(const dedaoapi::LiveDetail& data);
adapter::BaseVideoView convertVideoView(const dedaoapi::Article& article);
