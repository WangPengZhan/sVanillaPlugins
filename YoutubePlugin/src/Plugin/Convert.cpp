#include "Convert.h"
#include "Util/TimerUtil.h"
#include "YoutubePluginMessage.h"

std::string convertTimeFormat(const std::string& input)
{
    std::tm tm = {};
    std::istringstream ss(input);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");

    if (ss.fail())
    {
        return {};
    }

    std::ostringstream output;
    output << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return output.str();
}

adapter::VideoView convertVideoView(const youtubeapi::MainResponse& data)
{
    adapter::VideoView videoListView;

    adapter::BaseVideoView item;

    item.Identifier = data.videoDetails.videoId;
    item.AlternateId = data.videoDetails.videoId;
    item.Title = data.videoDetails.title;
    item.Publisher = data.videoDetails.author;
    item.Cover = data.videoDetails.thumbnails.empty() ? "" : data.videoDetails.thumbnails.back().url;
    try
    {
        item.Duration = formatDuration(std::stoll(data.videoDetails.lengthSeconds));
    }
    catch (const std::exception& e)
    {
        item.Duration = "0:00";
    }

    item.Description = data.videoDetails.shortDescription;
    item.PublishDate = convertTimeFormat(data.microformat.playerMicroformatRenderer.publishDate);
    item.pluginType = youtubelugin::pluginID;
    videoListView.push_back(item);

    return videoListView;
}