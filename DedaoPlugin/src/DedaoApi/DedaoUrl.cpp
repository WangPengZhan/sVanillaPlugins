#include "DedaoUrl.h"

#include <regex>
#include <sstream>

namespace
{
std::regex livePattern(R"(https?://(?:www\.)?dedao\.cn/live/detail\?id=([A-Za-z0-9]{20,100}))");
std::regex ebookPattern(R"(https?://(?:www\.)?dedao\.cn/ebook/reader\?id=([A-Za-z0-9]+))");
std::regex coursePattern(R"(https?://(?:www\.)?dedao\.cn/course/detail\?id=([A-Za-z0-9]+))");
std::regex articlePattern(R"(https?://(?:www\.)?dedao\.cn/course/article\?id=([A-Za-z0-9]+))");

std::vector<std::regex> validUrlPatterns = {livePattern, ebookPattern, coursePattern, articlePattern};

constexpr char unkown[] = "Unkown";
constexpr char live[] = "Live";
constexpr char eBook[] = "EBook";
constexpr char course[] = "Course";
constexpr char article[] = "Article";
}  // namespace

std::string typeToString(IDType type)
{
    switch (type)
    {
    case IDType::Unkown:
    {
        return unkown;
    }
    case IDType::Live:
    {
        return live;
    }
    case IDType::EBook:
    {
        return eBook;
    }
    case IDType::Course:
    {
        return course;
    }
    case IDType::Article:
    {
        return article;
    }
    default:
        break;
    }

    return {};
}

IDType stringToType(const std::string& type)
{
    if (type == unkown)
    {
        return IDType::Unkown;
    }
    else if (type == live)
    {
        return IDType::Live;
    }
    else if (type == eBook)
    {
        return IDType::EBook;
    }
    else if (type == course)
    {
        return IDType::Course;
    }
    else if (type == article)
    {
        return IDType::Article;
    }

    return IDType::Unkown;
}

std::string IDInfo::to_string() const
{
    std::stringstream ss;
    ss << "id: " << id << ", " << "id type: " << typeToString(type) << ", " << "parent id: " << parentId;
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
    try
    {
        if (!isValidUrl(url))
        {
            return {};
        }

        IDInfo id;

        std::smatch match;
        if (std::regex_match(url, match, livePattern))
        {
            id.id = match[1];
            id.type = IDType::Live;
            return id;
        }

        if (std::regex_search(url, match, ebookPattern))
        {
            id.id = match[1];
            id.type = IDType::EBook;
            return id;
        }

        if (std::regex_search(url, match, coursePattern))
        {
            id.id = match[1];
            id.type = IDType::Course;
            return id;
        }

        if (std::regex_search(url, match, articlePattern))
        {
            id.id = match[1];
            id.type = IDType::Article;
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
