#include "DouYinUrl.h"

#include <regex>
#include <sstream>

namespace douyinapi
{

constexpr char awemeId[] = "AwemeId";
constexpr char seriesId[] = "SeriesId";
constexpr char mixId[] = "MixId";
constexpr char userId[] = "UserId";

const std::regex accountLinkRegex(R"(\S*?https://www\.douyin\.com/user/([A-Za-z0-9_-]+)(?:\S*?\bmodal_id=(\d{19}))?)");
const std::regex accountShareRegex(R"(\S*?https://www\.iesdouyin\.com/share/user/(\S*?)\?\S*?)");
const std::regex detailLinkRegex(R"(\S*?https://www\.douyin\.com/(?:video|note|slides)/([0-9]{19})\S*?)");
const std::regex detailShareRegex(R"(\S*?https://www\.iesdouyin\.com/share/(?:video|note|slides)/([0-9]{19})/\S*?)");
const std::regex detailSearchRegex(R"(\S*?https://www\.douyin\.com/search/\S+?modal_id=(\d{19})\S*?)");
const std::regex detailDiscoverRegex(R"(\S*?https://www\.douyin\.com/discover\S*?modal_id=(\d{19})\S*?)");
const std::regex mixLinkRegex(R"(\S*?https://www\.douyin\.com/collection/(\d{19})\S*?)");
const std::regex mixShareRegex(R"(\S*?https://www\.iesdouyin\.com/share/mix/detail/(\d{19})/\S*?)");
const std::regex seriesShareRegex(R"(\S*?https://www\.iesdouyin\.com/share/series/detail/(\d{19})/\S*?)");
const std::regex liveLinkRegex(R"(\S*?https://live\.douyin\.com/([0-9]+)\S*?)");
const std::regex liveLinkSelfRegex(R"(\S*?https://www\.douyin\.com/follow\?webRid=(\d+)\S*?)");
const std::regex liveShareRegex(R"(https://webcast\.amemv\.com/douyin/webcast/reflow/\S+)");
const std::regex channelLinkRegex(R"(\S*?https://www\.douyin\.com/channel/\d+?\?modal_id=(\d{19})\S*?)");

const std::vector<std::regex> validUrlPatterns = {accountLinkRegex,    accountShareRegex, detailLinkRegex, detailShareRegex, detailSearchRegex,
                                                  detailDiscoverRegex, mixLinkRegex,      mixShareRegex,   seriesShareRegex, channelLinkRegex};

std::string typeToString(IDType type)
{
    switch (type)
    {
    case IDType::AwemeId:
        return awemeId;
    case IDType::SeriesId:
        return seriesId;
    case IDType::MixId:
        return mixId;
    case IDType::UserId:
        return userId;
    default:
        break;
    }

    return {};
}
IDType stringToType(const std::string& type)
{
    if (type == awemeId)
    {
        return IDType::AwemeId;
    }
    else if (type == seriesId)
    {
        return IDType::SeriesId;
    }
    else if (type == mixId)
    {
        return IDType::MixId;
    }
    else if (type == userId)
    {
        return IDType::UserId;
    }
    else
    {
        return IDType::Unkown;
    }
}

std::string IDInfo::to_string() const
{
    std::stringstream ss;
    ss << "id=" << id << ", type=" << typeToString(type) << ", parentId=" << parentId << ", parentIdType=" << typeToString(parentIdType);
    return ss.str();
}

bool isValidUrl(const std::string& url)
{
    try
    {
        for (const auto& pattern : validUrlPatterns)
        {
            if (std::regex_search(url, pattern))
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
    if (std::regex_search(url, match, accountLinkRegex))
    {
        id.id = match[1].str();
        id.type = IDType::UserId;
        if (match.size() > 2 && match[2].matched)
        {
            id.parentId = match[2].str();
            id.parentIdType = IDType::AwemeId;
        }
    }
    else if (std::regex_search(url, match, detailLinkRegex) || std::regex_search(url, match, detailShareRegex) ||
             std::regex_search(url, match, detailSearchRegex) || std::regex_search(url, match, detailDiscoverRegex) ||
             std::regex_search(url, match, channelLinkRegex))
    {
        id.id = match[1].str();
        id.type = IDType::AwemeId;
    }
    else if (std::regex_search(url, match, mixLinkRegex) || std::regex_search(url, match, mixShareRegex))
    {
        id.id = match[1].str();
        id.type = IDType::MixId;
    }
    else if (std::regex_search(url, match, seriesShareRegex))
    {
        id.id = match[1].str();
        id.type = IDType::SeriesId;
    }
    else if (std::regex_search(url, match, accountShareRegex))
    {
        id.id = match[1].str();
        id.type = IDType::UserId;
    }
    else
    {
        return {};
    }

    return id;
}

}  // namespace douyinapi
