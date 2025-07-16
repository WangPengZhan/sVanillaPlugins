#include "NeteaseCloudMusicUrl.h"

#include <regex>

#include "NeteaseCloudMusicApi/NeteaseCloudMusicApi.h"

namespace
{
std::regex songPattern(R"(https?://(y\.)?music\.163\.com/(?:[#m]/)?song\?.*?\bid=([0-9]+))");
std::regex albumPattern(R"(https?://music\.163\.com/(#/)?album\?id=([0-9]+))");
std::regex artistPattern(R"(https?://music\.163\.com/(#/)?artist\?id=([0-9]+))");
std::regex playlistPattern(R"(https?://music\.163\.com/(#/)?(playlist|discover/toplist)\?id=([0-9]+))");
std::regex mvPattern(R"(https?://music\.163\.com/(#/)?mv\?id=([0-9]+))");

std::vector<std::regex> validUrlPatterns = {songPattern, albumPattern, artistPattern, playlistPattern, mvPattern};

}  // namespace
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

std::string getID(const std::string& url)
{
    if (!isValidUrl(url))
    {
        return {};
    }

    std::smatch match;
    if (std::regex_match(url, match, songPattern))
    {
        std::string id = match[2];
        return netease::idTypeToString(netease::IDType::Song) + ":" + id;
    }

    if (std::regex_match(url, match, albumPattern))
    {
        std::string id = match[2];
        return netease::idTypeToString(netease::IDType::Album) + ":" + id;
    }

    if (std::regex_match(url, match, artistPattern))
    {
        std::string id = match[2];
        return netease::idTypeToString(netease::IDType::Artist) + ":" + id;
    }

    if (std::regex_match(url, match, playlistPattern))
    {
        std::string id = match[3];
        return netease::idTypeToString(netease::IDType::Playlist) + ":" + id;
    }

    if (std::regex_match(url, match, mvPattern))
    {
        std::string id = match[2];
        return netease::idTypeToString(netease::IDType::MV) + ":" + id;
    }

    return {};
}
