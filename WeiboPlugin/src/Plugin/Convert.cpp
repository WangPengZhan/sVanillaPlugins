#include "Convert.h"
#include "Util/TimerUtil.h"
#include "WeiboPluginMessage.h"

adapter::VideoView convertVideoView(const weiboapi::ComponentPlayPlayinfoResponse& data)
{
    adapter::VideoView views;

    adapter::BaseVideoView baseView;
    const auto& playInfo = data.data.Component_Play_Playinfo;
    baseView.Identifier = playInfo.id;
    baseView.AlternateId = playInfo.mid;
    baseView.VideoId = std::to_string(playInfo.media_id);
    baseView.Title = playInfo.title;
    baseView.Publisher = playInfo.author;
    baseView.Cover = "https:" + playInfo.cover_image;
    baseView.Duration = formatDuration(playInfo.duration_time);
    baseView.Description = playInfo.text;
    baseView.PublishDate = convertTimestamp(playInfo.real_date);
    baseView.PlayListTitle = playInfo.title;
    baseView.pluginType = weiboplugin::pluginID;

    views.push_back(baseView);

    return views;
}

adapter::VideoView convertVideoView(const weiboapi::WeiboAjaxData& data)
{
    adapter::VideoView views;

    for (const auto& item : data.mix_media_info.items)
    {
        if (!item.type.starts_with("video"))
        {
            continue;
        }
        adapter::BaseVideoView baseView = convertMediaInfoItem(item);
        views.push_back(baseView);
    }

    return views;
}

adapter::BaseVideoView convertMediaInfoItem(const weiboapi::MediaInfoItem& data)
{
    adapter::BaseVideoView baseView;

    baseView.Identifier = data.id;
    baseView.AlternateId = data.data.page_id;
    baseView.VideoId = data.data.media_info.media_id;
    baseView.Title = data.data.media_info.name + data.data.media_info.media_id;
    baseView.Publisher = data.data.media_info.author_name;
    baseView.Cover = data.data.media_info.big_pic_info.pic_big.url;
    baseView.Duration = formatDuration(data.data.media_info.duration);
    baseView.Description = data.data.content2;
    baseView.PublishDate = convertTimestamp(data.data.media_info.video_publish_time);
    baseView.pluginType = weiboplugin::pluginID;

    return baseView;
}
