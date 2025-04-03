#include "Convert.h"
#include "Util/TimerUtil.h"
#include "BilibiliPluginMessage.h"

adapter::VideoView convertVideoView(const biliapi::VideoView& data)
{
    adapter::VideoView videoListView;

    if (checkSeason(data))
    {
        const auto episodes = data.ugc_season.sections.front().episodes;
        for (const auto& e : episodes)
        {
            auto item = convertEpisodes(e);
            item.Publisher = data.owner.name;
            item.PlayListTitle = data.ugc_season.title;
            videoListView.push_back(item);
        }
        return videoListView;
    }
    else if (checkPages(data))
    {
        const auto pages = data.pages;
        for (const auto& p : pages)
        {
            auto item = convertPages(p);
            item.Identifier = data.bvid;
            videoListView.push_back(item);
        }
        return videoListView;
    }

    videoListView.push_back(convertSingleVideo(data));
    return videoListView;
}

adapter::VideoView convertVideoView(const biliapi::History& data)
{
    adapter::VideoView videoListView;
    videoListView.reserve(256);

    for (const auto& historyInfo : data.data.list)
    {
        videoListView.emplace_back(convertHistory(historyInfo));
    }

    return videoListView;
}

bool checkSeason(const biliapi::VideoView& data)
{
    if (data.ugc_season.sections.empty())
    {
        return false;
    }
    return !data.ugc_season.sections.front().episodes.empty();
}

bool checkPages(const biliapi::VideoView& data)
{
    return data.pages.size() > 1;
}

adapter::BaseVideoView convertEpisodes(const biliapi::UgcEpisode& data)
{
    adapter::BaseVideoView item;
    item.Identifier = data.bvid;
    item.AlternateId = std::to_string(data.aid);
    item.VideoId = std::to_string(data.cid);
    item.Title = data.title;
    item.Cover = data.arc.pic;
    item.Duration = formatDuration(data.page.duration);
    item.Description = !data.arc.desc.empty() ? data.arc.desc : data.page.part;
    item.PublishDate = convertTimestamp(data.arc.pubdate);
    item.pluginType = biliplugin::pluginID;

    return item;
}

adapter::BaseVideoView convertPages(const biliapi::VideoPage& data)
{
    adapter::BaseVideoView item;
    item.AlternateId = std::to_string(data.cid);
    item.VideoId = std::to_string(data.cid);
    item.Title = data.part;
    item.Cover = data.first_frame;
    item.Duration = formatDuration(data.duration);
    item.pluginType = biliplugin::pluginID;

    return item;
}

adapter::BaseVideoView convertSingleVideo(const biliapi::VideoView& data)
{
    adapter::BaseVideoView item;
    item.Identifier = data.bvid;
    item.AlternateId = std::to_string(data.aid);
    item.VideoId = std::to_string(data.cid);
    item.Title = data.title;
    item.Publisher = data.owner.name;
    item.Cover = data.pic;
    item.Duration = formatDuration(data.duration);
    item.Description = data.desc;
    item.PublishDate = convertTimestamp(data.pubdate);
    item.pluginType = biliplugin::pluginID;

    return item;
}

adapter::BaseVideoView convertHistory(const biliapi::HistoryInfo& data)
{
    adapter::BaseVideoView item;
    item.Identifier = data.history.bvid;
    item.AlternateId = std::to_string(data.history.cid);
    item.VideoId = std::to_string(data.history.cid);
    item.Title = data.title;
    item.Cover = data.cover.empty() ? (data.covers.empty() ? "" : data.covers.front()) : data.cover;
    item.Duration = formatDuration(data.duration);
    item.Description = data.new_desc;
    item.PublishDate = convertTimestamp(data.view_at);
    item.pluginType = biliplugin::pluginID;

    return item;
}
