#include "BilibiliUrl.h"

#include <regex>
#include <sstream>

namespace biliapi
{

constexpr char aid[] = "Aid";
constexpr char bid[] = "Bid";
constexpr char bangumiSS[] = "BangumiSS";
constexpr char bangumiEP[] = "BangumiEP";
constexpr char bangumiMD[] = "BangumiMD";
constexpr char cheeseSS[] = "CheeseSS";
constexpr char cheeseEP[] = "CheeseEP";
constexpr char favoritesId[] = "FavoritesId";
constexpr char userId[] = "UserId";

const std::regex avPattern(R"(https?://(?:www\.)?(?:bilibili\.com|b23\.tv)(?:/video)?/av(\d+).*)");
const std::regex bvPattern(R"(https?://(?:www\.)?(?:bilibili\.com|b23\.tv)(?:/video)?/BV([0-9A-Za-z]{10}).*)");
const std::regex bangumiSSPattern(R"(https?://(?:www\.)?bilibili\.com/bangumi/play/ss(\d+).*)");
const std::regex bangumiEPPattern(R"(https?://(?:www\.)?bilibili\.com/bangumi/play/ep(\d+).*)");
const std::regex bangumiMDPattern(R"(https?://(?:www\.)?bilibili\.com/bangumi/media/md(\d+).*)");
const std::regex cheeseSSPattern(R"(https?://(?:www\.)?bilibili\.com/cheese/play/ss(\d+).*)");
const std::regex cheeseEPPattern(R"(https?://(?:www\.)?bilibili\.com/cheese/play/ep(\d+).*)");
const std::regex favoritesPattern(R"(https?://(?:www\.)?bilibili\.com/medialist/(?:detail|play)/ml(\d+).*)");
const std::regex userIdPattern(R"(https?://(?:www\.)?space\.bilibili\.com/(\d+).*)");

std::vector<std::regex> validUrlPatterns = {avPattern,       bvPattern,       bangumiSSPattern, bangumiEPPattern, bangumiMDPattern,
                                            cheeseSSPattern, cheeseEPPattern, favoritesPattern, userIdPattern};

std::string typeToString(IDType type)
{
    switch (type)
    {
    case IDType::Aid:
        return aid;
    case IDType::Bid:
        return bid;
    case IDType::BangumiSS:
        return bangumiSS;
    case IDType::BangumiEP:
        return bangumiEP;
    case IDType::BangumiMD:
        return bangumiMD;
    case IDType::CheeseSS:
        return cheeseSS;
    case IDType::CheeseEP:
        return cheeseEP;
    case IDType::FavoritesId:
        return favoritesId;
    case IDType::UserId:
        return userId;
    default:
        break;
    }

    return {};
}

IDType stringToType(const std::string& type)
{
    if (type == aid)
    {
        return IDType::Aid;
    }
    else if (type == bid)
    {
        return IDType::Bid;
    }
    else if (type == bangumiSS)
    {
        return IDType::BangumiSS;
    }
    else if (type == bangumiEP)
    {
        return IDType::BangumiEP;
    }
    else if (type == bangumiMD)
    {
        return IDType::BangumiMD;
    }
    else if (type == cheeseSS)
    {
        return IDType::CheeseSS;
    }
    else if (type == cheeseEP)
    {
        return IDType::CheeseEP;
    }
    else if (type == favoritesId)
    {
        return IDType::FavoritesId;
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

std::string IDInfo::to_string() const
{
    std::stringstream ss;
    ss << "id: " << id << ", " << "id type: " << typeToString(type) << ", " << "parent id: " << parentId;
    return ss.str();
}

IDInfo getID(const std::string& url)
{
    try
    {
        if (!isValidUrl(url))
        {
            return {};
        }

        IDInfo id;

        std::smatch match;
        if (std::regex_match(url, match, avPattern))
        {
            id.id = match[1];
            id.type = IDType::Aid;
            return id;
        }

        if (std::regex_search(url, match, bvPattern))
        {
            id.id = match[1];
            id.type = IDType::Bid;
            return id;
        }

        if (std::regex_search(url, match, bangumiSSPattern))
        {
            id.id = match[1];
            id.type = IDType::BangumiSS;
            return id;
        }

        if (std::regex_search(url, match, bangumiEPPattern))
        {
            id.id = match[1];
            id.type = IDType::BangumiEP;
            return id;
        }

        if (std::regex_search(url, match, bangumiMDPattern))
        {
            id.id = match[1];
            id.type = IDType::BangumiMD;
            return id;
        }

        if (std::regex_search(url, match, cheeseSSPattern))
        {
            id.id = match[1];
            id.type = IDType::CheeseSS;
            return id;
        }

        if (std::regex_search(url, match, cheeseEPPattern))
        {
            id.id = match[1];
            id.type = IDType::CheeseEP;
            return id;
        }

        if (std::regex_search(url, match, favoritesPattern))
        {
            id.id = match[1];
            id.type = IDType::FavoritesId;
            return id;
        }

        if (std::regex_search(url, match, userIdPattern))
        {
            id.id = match[1];
            id.type = IDType::UserId;
            return id;
        }
    }
    catch (const std::regex_error& e)
    {
        int errorCode = e.code();
        std::string errorMessage = e.what();
    }

    return {};
}

}  // namespace biliapi
