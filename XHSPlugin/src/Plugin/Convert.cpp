#include "Convert.h"

#include "XHSPluginMessage.h"
#include "Util/TimerUtil.h"

std::string getVideoUrl(const std::vector<xhsapi::StreamItem>& stream)
{
    const auto isBetterStream = [](const xhsapi::StreamItem& item, const xhsapi::StreamItem* selected) {
        return !selected || item.height > selected->height || (item.height == selected->height && item.video_bitrate > selected->video_bitrate);
    };

    const xhsapi::StreamItem* selected = nullptr;
    // Prefer streams that expose a master url; backup urls are a fallback only.
    for (const auto& item : stream)
    {
        if (item.master_url.empty())
        {
            continue;
        }
        if (isBetterStream(item, selected))
        {
            selected = &item;
        }
    }

    if (!selected)
    {
        for (const auto& item : stream)
        {
            if (item.backup_urls.empty())
            {
                continue;
            }
            if (isBetterStream(item, selected))
            {
                selected = &item;
            }
        }
    }

    if (!selected)
    {
        return "";
    }

    return selected->master_url.empty() ? selected->backup_urls.front() : selected->master_url;
}

std::string getVideoUrl(const xhsapi::Media& media)
{
    std::string videoUrl;
    videoUrl = getVideoUrl(media.stream.h264);
    if (!videoUrl.empty())
    {
        return videoUrl;
    }

    videoUrl = getVideoUrl(media.stream.h265);
    if (!videoUrl.empty())
    {
        return videoUrl;
    }

    videoUrl = getVideoUrl(media.stream.h266);
    if (!videoUrl.empty())
    {
        return videoUrl;
    }

    videoUrl = getVideoUrl(media.stream.av1);
    if (!videoUrl.empty())
    {
        return videoUrl;
    }

    return videoUrl;
}

adapter::BaseVideoView convertNoteDetail(const xhsapi::NoteItemInfo& note)
{
    adapter::BaseVideoView view;

    view.Identifier = note.note_id;
    view.Title = note.display_title;
    view.Publisher = note.user.nickname;
    view.Cover = note.cover.url_default;
    view.pluginId = xhsplugin::pluginID;
    // Note resources use Option1 as the optional xsec_token required when the
    // detail endpoint is queried again for downloading.
    view.Option1 = note.xsec_token;

    return view;
}

adapter::BaseVideoView convertNoteDetail(const xhsapi::NoteCard& note)
{
    adapter::BaseVideoView view;

    view.Identifier = note.note_id;
    view.Title = note.title;
    view.Publisher = note.user.nickname;
    view.Cover = note.image_list.empty() ? "" : note.image_list.front().url_default;
    view.Duration = formatDuration(note.video.capa.duration);
    view.Description = note.desc;
    view.PublishDate = convertTimestamp(note.time);
    view.pluginId = xhsplugin::pluginID;

    return view;
}

adapter::VideoView convertNoteDetail(const xhsapi::NoteItemList& note)
{
    adapter::VideoView views;
    views.reserve(note.notes.size());

    for (const auto& item : note.notes)
    {
        views.push_back(convertNoteDetail(item));
    }

    return views;
}
