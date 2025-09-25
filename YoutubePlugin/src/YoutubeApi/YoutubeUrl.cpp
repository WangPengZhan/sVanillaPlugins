#include "YoutubeUrl.h"
#include "Util/UrlProccess.h"

#include <regex>
#include <sstream>
namespace youtubeapi
{

constexpr char videoId[] = "VideoId";
constexpr char playlistId[] = "PlaylistId";
constexpr char channelId[] = "ChannelId";
constexpr char atChannelId[] = "AtChannelId";
constexpr char userNameId[] = "UserNameId";
constexpr char customNameId[] = "CustomNameId";

const std::regex ytVideoPlaylistPattern(
    R"(https?://(?:www\.)?(?:youtube\.com/(?:watch\?v=|embed/)|youtu\.be/)([a-zA-Z0-9_-]+)(?:/)?(?:\?list=|&list=)([a-zA-Z0-9_-]+)(?:\S*)?)");
const std::regex ytPattern(R"(https?://(?:www\.youtube\.com|youtu\.be|music\.youtube\.com|www\.youtubekids\.com)/watch\?v=([a-zA-Z0-9_-]+)(?:\S*)?)");
const std::regex ytShortPattern(R"(https?://youtu\.be/([a-zA-Z0-9_-]+)(?:\S*)?)");
const std::regex ytEmbedPattern(R"(https?://www\.youtube\.com/embed/([a-zA-Z0-9_-]+)(?:\S*)?)");
const std::regex ytShortsPattern(R"(https?://(?:www\.)?youtube\.com/shorts/([a-zA-Z0-9_-]+)(?:\S*)?)");
const std::regex ytChannelPattern(R"(https?://(?:www\.)?youtube\.com/channel/([a-zA-Z0-9_-]+)(?:\S*)?)");
const std::regex ytPlaylistPattern(R"(https?://(?:www\.)?youtube\.com/playlist\?list=([a-zA-Z0-9_-]+)(?:\S*)?)");
const std::regex ytAtChannelPattern(R"(https?://(?:www\.)?youtube\.com/@([A-Za-z0-9_]+)(?:\S*)?)");
const std::regex ytUserNamePattern(R"(https?://(?:www\.)?youtube\.com/user/([A-Za-z0-9_]+)(?:\S*)?)");
const std::regex ytCustomNamePattern(R"(https?://(?:www\.)?youtube\.com/c/([A-Za-z0-9_]+)(?:\S*)?)");

std::vector<std::regex> validUrlPatterns = {ytPattern,         ytShortPattern,         ytEmbedPattern,     ytShortsPattern,   ytChannelPattern,
                                            ytPlaylistPattern, ytVideoPlaylistPattern, ytAtChannelPattern, ytUserNamePattern, ytCustomNamePattern};
std::string typeToString(IDType type)
{
    switch (type)
    {
    case IDType::VideoId:
        return videoId;
    case IDType::PlaylistId:
        return playlistId;
    case IDType::ChannelId:
        return channelId;
    case IDType::AtChannelId:
        return atChannelId;
    case IDType::UserNameId:
        return userNameId;
    case IDType::CustomNameId:
        return customNameId;
    default:
        break;
    }
    return {};
}
IDType stringToType(const std::string& type)
{
    if (type == videoId)
    {
        return IDType::VideoId;
    }
    else if (type == playlistId)
    {
        return IDType::PlaylistId;
    }
    else if (type == channelId)
    {
        return IDType::ChannelId;
    }
    else if (type == atChannelId)
    {
        return IDType::AtChannelId;
    }
    else if (type == userNameId)
    {
        return IDType::UserNameId;
    }
    else if (type == customNameId)
    {
        return IDType::CustomNameId;
    }
    else
    {
        return IDType::Unkown;
    }
}

bool isValidUrl(const std::string& url)
{
    for (const auto& pattern : validUrlPatterns)
    {
        if (std::regex_match(url, pattern))
        {
            return true;
        }
    }

    return false;
}

static bool isValidID(const std::string& videoId)
{
    static const std::regex idPattern(R"(^[a-zA-Z0-9_-]{11}$)");
    return std::regex_match(videoId, idPattern);
}

IDInfo getID(const std::string& url)
{
    if (!isValidUrl(url))
    {
        return {};
    }
    IDInfo id;
    std::smatch match;
    if (std::regex_search(url, match, ytVideoPlaylistPattern))
    {
        id.id = match[1].str();
        id.type = IDType::VideoId;
        id.parentId = match[2].str();
        id.parentType = IDType::PlaylistId;
        return id;
    }
    if (std::regex_search(url, match, ytPattern))
    {
        id.id = match[1].str();
        id.type = IDType::VideoId;
        return id;
    }
    if (std::regex_search(url, match, ytShortPattern))
    {
        id.id = match[1].str();
        id.type = IDType::VideoId;
        return id;
    }
    if (std::regex_search(url, match, ytEmbedPattern))
    {
        id.id = match[1].str();
        id.type = IDType::VideoId;
        return id;
    }
    if (std::regex_search(url, match, ytShortsPattern))
    {
        id.id = match[1].str();
        id.type = IDType::VideoId;
        return id;
    }
    if (std::regex_search(url, match, ytPlaylistPattern))
    {
        id.id = match[1].str();
        id.type = IDType::PlaylistId;
        return id;
    }
    if (std::regex_search(url, match, ytChannelPattern))
    {
        id.id = match[1].str();
        id.type = IDType::ChannelId;
        return id;
    }
    if (std::regex_search(url, match, ytAtChannelPattern))
    {
        id.id = match[1].str();
        id.type = IDType::AtChannelId;
        return id;
    }
    if (std::regex_search(url, match, ytUserNamePattern))
    {
        id.id = match[1].str();
        id.type = IDType::UserNameId;
        return id;
    }
    if (std::regex_search(url, match, ytCustomNamePattern))
    {
        id.id = match[1].str();
        id.type = IDType::CustomNameId;
        return id;
    }
    return {};
}
}  // namespace youtubeapi
