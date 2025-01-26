#include "YoutubeUrl.h"

#include <regex>

bool isValidUrl(const std::string& url)
{
    std::regex bilibiliPattern(R"((http|https):\/\/((www|m|space)\.)?(youtube\.com|b23\.tv))");
    return std::regex_search(url, bilibiliPattern);
}

std::string getID(const std::string& url)
{
    if (!isValidUrl(url))
    {
        return {};
    }

    std::regex bvPattern(R"(v=([A-Za-z0-9]{11}))");

    std::smatch match;
    if (std::regex_search(url, match, bvPattern))
    {
        return match[1].str();
    }

    bvPattern = std::regex(R"(youtu\.be/(.*?)(?:\?|&|/|$))");
    if (std::regex_search(url, match, bvPattern))
    {
        return match[1].str();
    }

    bvPattern = std::regex(R"(youtube\..+?/embed/(.*?)(?:\?|&|/|$)))");
    if (std::regex_search(url, match, bvPattern))
    {
        return match[1].str();
    }

    bvPattern = std::regex(R"(youtube\..+?/shorts/(.*?)(?:\?|&|/|$))");
    if (std::regex_search(url, match, bvPattern))
    {
        return match[1].str();
    }

    bvPattern = std::regex(R"((youtube\..+?/live/(.*?)(?:\?|&|/|$))");
    if (std::regex_search(url, match, bvPattern))
    {
        return match[1].str();
    }


    return "";
}