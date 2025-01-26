#pragma once
#include <string>

namespace youtubeapi
{
inline const std::string youtubeMainUrl = "https://www.youtube.com";
inline const std::string youtubePlayerUrl = youtubeMainUrl + "/youtubei/v1/player";
inline const std::string youtubeIFrame = youtubeMainUrl + "/iframe_api";
inline const constexpr char youtubeBaseJs[] = "https://www.youtube.com/s/player/{}/player_ias.vflset/en_US/base.js";

constexpr char youtubePostContent[] = R"delimiter(       
    {
        "videoId": null,
        "contentCheckOk": true,
        "context": {
            "client": {
                "clientName": "1",
                "clientVersion": "2.20250108.06.00",
                "deviceMake": "Apple",
                "deviceModel": "iPhone16,2",
                "hl": "en",
                "osName": "iPhone",
                "osVersion": "17.5.1.21F90",
                "timeZone": "UTC",
                "userAgent": "com.google.ios.youtube/19.29.1 (iPhone16,2; U; CPU iOS 17_5_1 like Mac OS X;)",
                "gl": "US",
                "utcOffsetMinutes": 0
            }
        }
    })delimiter";

constexpr char youtubeMustKey[] = "LOGIN_INFO";
}  // namespace youtubeapi