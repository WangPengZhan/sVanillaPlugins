#include <sstream>

#include "Convert.h"
#include "Util/TimerUtil.h"
#include "NeteaseCloudMusicPluginMessage.h"

#include <iomanip>

std::string authorFormat(const std::vector<netease::ArtistRef> artists)
{
    std::string result;
    for (const auto& artist : artists)
    {
        result += artist.name;
        result += ";";
    }

    if (!result.empty())
    {
        result.pop_back();
    }

    return result;
}

adapter::VideoView convertVideoView(const netease::SongDetails& data)
{
    adapter::VideoView views;

    for (const auto& song : data.songs)
    {
        adapter::BaseVideoView view;
        view.Identifier = std::to_string(int(ContentType::SONG));
        view.VideoId = std::to_string(song.id);
        view.Title = song.name;
        view.Publisher = authorFormat(song.ar);
        view.Cover = song.al.picUrl;
        view.Duration = formatDuration(song.dt / 1000);
        view.Description = song.rt;
        view.PublishDate = convertTimestamp(song.publishTime / 1000);
        view.PlayListTitle = "";
        view.fileExtension = ".mp3";
        view.pluginType = netease::pluginID;
        views.push_back(view);
    }

    return views;
}

adapter::VideoView convertVideoView(const netease::MVResponse& data)
{
    adapter::VideoView views;

    adapter::BaseVideoView view;
    view.Identifier = std::to_string(int(ContentType::MV));
    view.VideoId = std::to_string(data.data.id);
    view.Title = data.data.name;
    view.Publisher = authorFormat(data.data.artists);
    view.Cover = data.data.cover;
    view.Duration = formatDuration(data.data.duration / 1000);
    view.PublishDate = data.data.publishTime;
    view.PlayListTitle = "";
    view.pluginType = netease::pluginID;
    views.push_back(view);

    return views;
}
