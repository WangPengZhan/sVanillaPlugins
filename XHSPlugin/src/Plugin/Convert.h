#pragma once

#include <BaseVideoView.h>
#include "XHSApi/XHSApi.h"

std::string getVideoUrl(const xhsapi::Media& media);

adapter::BaseVideoView convertNoteDetail(const xhsapi::NoteItemInfo& note);

adapter::BaseVideoView convertNoteDetail(const xhsapi::NoteCard& note);

adapter::VideoView convertNoteDetail(const xhsapi::NoteItemList& note);
