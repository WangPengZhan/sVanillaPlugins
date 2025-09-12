#include "Convert.h"
#include "Util/TimerUtil.h"
#include "DedaoPluginMessage.h"

std::string authorFormat(const std::vector<dedaoapi::Teacher> teachers)
{
    std::string result;
    for (const auto& teacher : teachers)
    {
        result += teacher.teacher_name;
        result += ";";
    }

    if (!result.empty())
    {
        result.pop_back();
    }

    return result;
}

adapter::VideoView convertVideoView(const dedaoapi::LiveDetail& data)
{
    adapter::VideoView views;
    adapter::BaseVideoView view;

    view.Title = data.title;
    view.Publisher = authorFormat(data.live_lecture_list);
    view.Cover = data.logo;
    view.Duration = data.playback_duration_text;
    view.Description = data.summary;
    view.PublishDate = data.last_end_time;
    view.pluginId = dedaoplugin::pluginID;
    views.push_back(std::move(view));

    return views;
}

adapter::BaseVideoView convertVideoView(const dedaoapi::Article& article)
{
    adapter::BaseVideoView view;

    view.Identifier = article.enid;
    view.ParentId = article.class_enid;
    view.Option1 = article.audio.mp3_play_url;
    view.Title = article.title;
    view.Cover = article.logo;
    view.Duration = formatDuration(article.audio.duration);
    view.Description = article.summary;
    view.PublishDate = convertTimestamp(article.update_time);
    view.pluginId = dedaoplugin::pluginID;

    return view;
}
