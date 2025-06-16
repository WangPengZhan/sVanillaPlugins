#include "WeiboUrl.h"

#include <regex>

bool isValidUrl(const std::string& url)
{
    try
    {
        std::regex weiboPattern(
            R"(https?://(?:(?:m\.weibo\.cn/(?:status|detail)|(?:www\.)?weibo\.com/\d+)/([a-zA-Z0-9]+)|(?:video\.weibo\.com/show\?fid=|weibo\.com/tv/show/|h5\.video\.weibo\.com/show/)(\d+:\d+)))");
        return std::regex_search(url, weiboPattern);
    }
    catch (const std::regex_error& e)
    {
        int errorCode = e.code();
        std::string errorMessage = e.what();
        return false;
    }
}

std::string getID(const std::string& url)
{
    try
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

        std::regex tvPattern(R"(https?://(?:video\.weibo\.com/show\?fid=|weibo\.com/tv/show/|h5\.video\.weibo\.com/show/)(\d+:[a-fA-f0-9]+))");
        if (std::regex_search(url, match, tvPattern))
        {
            std::string id = match[1];
            if (id.find(":") != std::string::npos)
            {
                id = id.substr(id.find(":") + 1);
            }
            return "mid:" + id;
        }
    }
    catch (const std::regex_error& e)
    {
        int errorCode = e.code();
        std::string errorMessage = e.what();
    }
    return "";
}
