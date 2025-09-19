#pragma once
#include <string>

namespace youtubeapi
{
constexpr char domain[] = ".youtube.com";

inline const std::string youtubeMainUrl = "https://www.youtube.com";
inline const std::string youtubePlayerUrl = youtubeMainUrl + "/youtubei/v1/player";
inline const std::string youtubeIFrame = youtubeMainUrl + "/iframe_api";
inline const constexpr char youtubeBaseJs[] = "https://www.youtube.com/s/player/{}/player_ias.vflset/en_US/base.js";
inline const std::string swJsDataUrl = "https://www.youtube.com/sw.js_data";

constexpr char youtubePostContent[] = R"delimiter(       
    {
        "videoId": null,
        "contentCheckOk": true,
        "context": {
            "client": {
                "clientName": "IOS",
                "clientVersion": "19.45.4",
                "deviceMake": "Apple",
                "deviceModel": "iPhone16,2",
                "platform": "MOBILE",
                "osName": "IOS",
                "osVersion": "18.1.0.22B83",
                "visitorData": null,
                "hl": "en",
                "gl": "US",
                "utcOffsetMinutes": 0
            }
        }
    })delimiter";

constexpr char youtubeMustKey[] = "LOGIN_INFO";

constexpr char youtube_origin[] = "Origin: https://www.youtube.com";
constexpr char youtube_referer[] = "Referer: https://www.youtube.com";
inline const std::string youtube_accept_encoding = "Accept-Encoding: gzip, deflate, br, zstd";
inline const std::string youtube_accept_language = "Accept-Language: en,zh-CN;q=0.9,zh;q=0.8";
inline const std::string youtube_user_agent = "User-Agent: com.google.ios.youtube/19.45.4 (iPhone16,2; U; CPU iOS 18_1_0 like Mac OS X; US)";
inline const std::string youtube_default_cookies = "SOCS=CAISEwgDEgk2NzM5OTg2ODUaAmVuIAEaBgiA6p23Bg; domain=.youtube.com";
}  // namespace youtubeapi
