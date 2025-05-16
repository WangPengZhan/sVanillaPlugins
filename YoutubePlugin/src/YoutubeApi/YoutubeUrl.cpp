#include "YoutubeUrl.h"
#include "Util/UrlProccess.h"

#include <regex>

bool isValidUrl(const std::string& url)
{
    std::regex youtubePattern(R"((http|https):\/\/((www|m|space)\.)?(youtu\.be|youtube\.com|b23\.tv))");
    return std::regex_search(url, youtubePattern);
}

static bool isValidID(const std::string& videoId)
{
    static const std::regex idPattern(R"(^[a-zA-Z0-9_-]{11}$)");
    return std::regex_match(videoId, idPattern);
}

std::string getID(const std::string& url)
{
    if (!isValidUrl(url))
    {
        return {};
    }

    static const std::vector<std::regex> patterns = {std::regex(R"((?:v=)?([a-zA-Z0-9_-]{11})$)"),   std::regex(R"(youtube\..+?/watch.*?v=([^&?/]+))"),
                                                     std::regex(R"(youtu\.be/([^&?/]+))"),           std::regex(R"(youtube\..+?/embed/([^&?/]+))"),
                                                     std::regex(R"(youtube\..+?/shorts/([^&?/]+))"), std::regex(R"(youtube\..+?/live/([^&?/]+))")};

    for (const auto& pattern : patterns)
    {
        std::smatch match;
        if (std::regex_search(url, match, pattern))
        {
            std::string id = util::urlDecode(match[1].str());
            if (isValidID(id))
                return id;
        }
    }

    return {};
}