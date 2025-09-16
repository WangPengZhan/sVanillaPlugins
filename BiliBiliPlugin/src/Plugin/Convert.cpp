#include "Convert.h"
#include "Util/TimerUtil.h"
#include "BiliBiliPluginMessage.h"
#include "BiliApi/BilibiliUrl.h"

// Identifier => bvid
// Option1 = aid
// Option2 = cid
// Option3 = bvid

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

adapter::VideoView convertVideoView(const biliapi::BangumiData& data)
{
    adapter::VideoView videoListView;
    videoListView.reserve(30);

    for (const auto& episode : data.episodes)
    {
        videoListView.emplace_back(convertEpisodes(episode));
    }

    for (const auto& section : data.section)
    {
        for (const auto& episode : section.episodes)
        {
            videoListView.emplace_back(convertEpisodes(episode));
        }
    }

    for (auto& view : videoListView)
    {
        view.Publisher = data.up_info.uname;
    }

    return videoListView;
}

adapter::VideoView convertVideoView(const biliapi::CheeseInfo& data)
{
    adapter::VideoView videoListView;
    videoListView.reserve(30);

    for (const auto& episode : data.episodes)
    {
        videoListView.emplace_back(convertEpisodes(episode));
    }

    for (auto& view : videoListView)
    {
        view.Publisher = data.up_info.uname;
    }

    return videoListView;
}

adapter::VideoView convertVideoView(const biliapi::VideoWorks& data)
{
    adapter::VideoView videoListView;
    videoListView.reserve(30);

    for (const auto& item : data.list.vlist)
    {
        videoListView.emplace_back(convertVideoInfo(item));
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
    item.IdType = biliapi::typeToString(biliapi::IDType::Bid);
    item.Option1 = std::to_string(data.aid);
    item.Option2 = std::to_string(data.cid);
    item.Title = data.title;
    item.Cover = data.arc.pic;
    item.Duration = formatDuration(data.page.duration);
    item.Description = !data.arc.desc.empty() ? data.arc.desc : data.page.part;
    item.PublishDate = convertTimestamp(data.arc.pubdate);
    item.pluginId = biliplugin::pluginID;

    return item;
}

adapter::BaseVideoView convertPages(const biliapi::VideoPage& data)
{
    adapter::BaseVideoView item;
    item.IdType = biliapi::typeToString(biliapi::IDType::Bid);
    item.Option1 = std::to_string(data.cid);
    item.Option2 = std::to_string(data.cid);
    item.Title = data.part;
    item.Cover = data.first_frame;
    item.Duration = formatDuration(data.duration);
    item.pluginId = biliplugin::pluginID;

    return item;
}

adapter::BaseVideoView convertSingleVideo(const biliapi::VideoView& data)
{
    adapter::BaseVideoView item;
    item.Identifier = data.bvid;
    item.IdType = biliapi::typeToString(biliapi::IDType::Bid);
    item.Option1 = std::to_string(data.aid);
    item.Option2 = std::to_string(data.cid);
    item.Title = data.title;
    item.Publisher = data.owner.name;
    item.Cover = data.pic;
    item.Duration = formatDuration(data.duration);
    item.Description = data.desc;
    item.PublishDate = convertTimestamp(data.pubdate);
    item.pluginId = biliplugin::pluginID;

    return item;
}

adapter::BaseVideoView convertHistory(const biliapi::HistoryInfo& data)
{
    adapter::BaseVideoView item;
    item.Identifier = data.history.bvid;
    item.IdType = biliapi::typeToString(biliapi::IDType::Bid);
    item.Option1 = std::to_string(data.history.cid);
    item.Option2 = std::to_string(data.history.cid);
    item.Title = data.title;
    item.Cover = data.cover.empty() ? (data.covers.empty() ? "" : data.covers.front()) : data.cover;
    item.Duration = formatDuration(data.duration);
    item.Description = data.new_desc;
    item.PublishDate = convertTimestamp(data.view_at);
    item.pluginId = biliplugin::pluginID;

    return item;
}

adapter::BaseVideoView convertEpisodes(const biliapi::Episode& data)
{
    adapter::BaseVideoView item;
    item.Identifier = std::to_string(data.ep_id);
    item.IdType = biliapi::typeToString(biliapi::IDType::BangumiEP);
    item.Option1 = std::to_string(data.aid);
    item.Option2 = std::to_string(data.cid);
    item.Title = data.title;
    item.Cover = data.cover;
    item.Duration = formatDuration(data.duration);
    item.Description = data.share_copy;
    item.PublishDate = convertTimestamp(data.pub_time);
    item.Option3 = data.bvid;
    item.pluginId = biliplugin::pluginID;

    return item;
}

adapter::BaseVideoView convertEpisodes(const biliapi::CheeseEpisode& data)
{
    adapter::BaseVideoView item;

    item.Identifier = std::to_string(data.id);
    item.IdType = biliapi::typeToString(biliapi::IDType::CheeseEP);
    item.Option1 = std::to_string(data.aid);
    item.Option2 = std::to_string(data.cid);
    item.Title = data.title;
    item.Cover = data.cover;
    item.Duration = formatDuration(data.duration);
    item.Description = data.subtitle;
    item.PublishDate = convertTimestamp(data.release_date);
    item.pluginId = biliplugin::pluginID;

    return item;
}

adapter::BaseVideoView convertVideoInfo(const biliapi::FavVideoInfo& data)
{
    adapter::BaseVideoView item;

    item.Identifier = data.bvid;
    item.IdType = biliapi::typeToString(biliapi::IDType::Bid);
    item.Option2 = std::to_string(data.ugc.first_cid);
    item.Title = data.title;
    item.Cover = data.cover;
    item.Duration = formatDuration(data.duration);
    item.Description = data.intro;
    item.PublishDate = convertTimestamp(data.pubtime);
    item.pluginId = biliplugin::pluginID;
    item.Publisher = data.upper.name;

    return item;
}

adapter::BaseVideoView convertVideoInfo(const biliapi::VlistItem& data)
{
    adapter::BaseVideoView item;
    item.Identifier = data.bvid;
    item.IdType = biliapi::typeToString(biliapi::IDType::Bid);  // item.Option2 = std::to_string(data.ugc.first_cid);
    item.Title = data.title;
    item.Cover = data.pic;
    item.Duration = data.length;
    item.Description = data.description;
    item.PublishDate = convertTimestamp(data.created);
    item.pluginId = biliplugin::pluginID;
    item.Publisher = data.author;
    return item;
}
