#include "XHSUrl.h"

#include <regex>
#include <sstream>

namespace xhsapi
{

constexpr char noteId[] = "NoteId";
constexpr char userId[] = "UserId";

const std::regex noteRegex(R"(https?://(?:www\.)?xiaohongshu\.com/(?:discovery/item|note|explore)/([0-9a-fA-F]+)[^?]*\?[^#]*xsec_token=([^&]+))");
const std::regex userRegex(R"(https?://(?:www\.)?xiaohongshu\.com/user/profile/([0-9a-fA-F]+)[^?]*\?[^#]*xsec_token=([^&]+))");

std::vector<std::regex> validUrlPatterns = {noteRegex, userRegex};

std::string typeToString(IDType type)
{
    switch (type)
    {
    case IDType::NoteId:
        return noteId;
    case IDType::UserId:
        return userId;
    default:
        break;
    }

    return {};
}
IDType stringToType(const std::string& type)
{
    if (type == noteId)
    {
        return IDType::NoteId;
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
    ss << "id=" << id << ", type=" << typeToString(type) << ", hasXsecToken=" << std::boolalpha << !xsecToken.empty() << ", parentId=" << parentId
       << ", parentIdType=" << typeToString(parentIdType) << ", hasParentXsecToken=" << !parentXsecToken.empty();
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
    if (std::regex_search(url, match, noteRegex))
    {
        id.id = match[1].str();
        id.type = IDType::NoteId;
        id.xsecToken = match[2].str();
    }
    else if (std::regex_search(url, match, userRegex))
    {
        id.id = match[1].str();
        id.type = IDType::UserId;
        id.xsecToken = match[2].str();
    }
    else
    {
        return {};
    }

    return id;
}

}  // namespace xhsapi
