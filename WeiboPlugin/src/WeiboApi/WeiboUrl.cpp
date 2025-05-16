#include "WeiboUrl.h"

#include <regex>

bool isValidUrl(const std::string& url)
{
    std::regex weiboPattern(R"(https:\/\/(?:weibo\.com(?:\/(?:u|tv\/show|\d+\/\d+))?|m\.weibo\.cn\/(?:status|detail))\/[\w:]+))");
    return std::regex_search(url, weiboPattern);
}

std::string getID(const std::string& url)
{
    if (!isValidUrl(url))
    {
        return {};
    }

    std::regex widPattern(R"(https?://(?:m\.weibo\.cn/(?:status|detail)|(?:www\.)?weibo\.com/\d+)/([a-zA-Z0-9]+))");
    std::smatch match;
    if (std::regex_match(url, match, widPattern))
    {
        std::string id = match[1];
        return "wid:" + id;
    }

    std::regex tvPattern(R"(https?://(?:video\.weibo\.com/show\?fid=|weibo\.com/tv/show/|h5\.video\.weibo\.com/show/)(\d+:\d+))");
    if (std::regex_search(url, match, tvPattern))
    {
        std::string id = match[1];
        return "mid:" + id;
    }

    return "";
}