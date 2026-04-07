#include "Convert.h"

#include "DouYinApi/DouYinUrl.h"
#include "DouYinPluginMessage.h"
#include "Util/TimerUtil.h"

std::string getCover(const douyinapi::Video& video)
{
    if (!video.origin_cover.url_list.empty())
    {
        return video.cover.url_list[0];
    }
    else if (!video.cover_original_scale.url_list.empty())
    {
        return video.cover_original_scale.url_list[0];
    }
    else if (!video.cover.url_list.empty())
    {
        return video.cover.url_list[0];
    }
    else
    {
        return video.cover.uri;
    }
}

std::string getPlayUrl(const douyinapi::Video& video)
{
    std::string url;
    int bitRate = 0;
    for (const auto& videoInfo : video.bit_rate)
    {
        if (videoInfo.bit_rate > bitRate)
        {
            bitRate = videoInfo.bit_rate;
            url = videoInfo.play_addr.url_list[0];
        }
    }
    return url;
}

douyinapi::FollowingUser getUserInfo(const douyinapi::FollowingResponse& following)
{
    for (const auto& user : following.followings)
    {
        if (user.sec_uid == following.owner_sec_uid)
        {
            return user;
        }
    }
    return {};
}

adapter::BaseVideoView convertAwemeDetail(const douyinapi::AwemeDetail& detail)
{
    adapter::BaseVideoView view;

    view.Identifier = detail.aweme_id;
    view.IdType = douyinapi::typeToString(douyinapi::IDType::AwemeId);
    view.Title = detail.item_title;
    view.Publisher = detail.author.nickname;
    view.Cover = getCover(detail.video);
    view.Duration = formatDuration(detail.duration);
    view.Description = detail.desc;
    view.PublishDate = convertTimestamp(detail.create_time);
    view.pluginId = douyinplugin::pluginID;

    return view;
}

adapter::VideoView convertSeriesDetail(const douyinapi::SeriesDetail& detail)
{
    adapter::VideoView views;

    for (const auto& aweme : detail.aweme_list)
    {
        views.push_back(convertAwemeDetail(aweme));
    }

    return views;
}
