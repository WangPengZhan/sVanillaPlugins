#include "Convert.h"

#include "XHSPluginMessage.h"
#include "Util/TimerUtil.h"

std::string getVideoUrl(const std::vector<xhsapi::StreamItem>& stream)
{
    if (!stream.empty())
    {
        if (!stream.front().master_url.empty())
        {
            return stream.front().master_url;
        }
    }

    return "";
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
    view.IdType;
    view.ParentId;
    view.ParentIdType;
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

    for (const auto& note : note.notes)
    {
        views.push_back(convertNoteDetail(note));
    }

    return views;
}
