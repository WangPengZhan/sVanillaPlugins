#include "WeiboUrl.h"

#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace weiboapi
{

constexpr char statusIdType[] = "Status";
constexpr char tvIdType[] = "TV";

std::regex widPattern(R"(https?://(?:m\.weibo\.cn/(?:status|detail)|(?:www\.)?weibo\.com/\d+)/([a-zA-Z0-9]+)(?:[/?&#][^\s]*)?)");
std::regex tvPattern(R"(https?://(?:video\.weibo\.com/show\?fid=|weibo\.com/tv/show/|h5\.video\.weibo\.com/show/)(\d+:[a-fA-F0-9]+)(?:[/?&#][^\s]*)?)");

std::vector<std::regex> validUrlPatterns = {widPattern, tvPattern};

std::string typeToString(IDType type)
{
    switch (type)
    {
    case IDType::Status:
        return statusIdType;
    case IDType::TV:
        return tvIdType;
    default:
        break;
    }

    return {};
}

IDType stringToType(const std::string& type)
{
    if (type == statusIdType)
    {
        return IDType::Status;
    }
    else if (type == tvIdType)
    {
        return IDType::TV;
    }
    else
    {
        return IDType::Unkown;
    }
}

std::string IDInfo::to_string() const
{
    std::stringstream ss;
    ss << "id=" << id << ", type=" << typeToString(type);
    return ss.str();
}

bool isValidUrl(const std::string& url)
{
    try
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
    catch (const std::regex_error& e)
    {
        int errorCode = e.code();
        std::string errorMessage = e.what();
        return false;
    }
}

IDInfo getID(const std::string& url)
{
    if (!isValidUrl(url))
    {
        return {};
    }

    IDInfo id;
    std::smatch match;

    if (std::regex_match(url, match, widPattern))
    {
        id.id = match[1].str();
        id.type = IDType::Status;
        return id;
    }

    if (std::regex_match(url, match, tvPattern))
    {
        std::string mid = match[1].str();
        const auto separator = mid.find(':');
        if (separator != std::string::npos)
        {
            mid = mid.substr(separator + 1);
        }

        id.id = mid;
        id.type = IDType::TV;
        return id;
    }

    return {};
}

}  // namespace weiboapi
