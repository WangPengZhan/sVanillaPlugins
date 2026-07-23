#pragma once
#include <string>

namespace youtubeapi
{
constexpr char domain[] = ".youtube.com";

inline const std::string youtubeMainUrl = "https://www.youtube.com";
inline const std::string youtubePlayerUrl = youtubeMainUrl + "/youtubei/v1/player";
inline const std::string youtubeIFrame = youtubeMainUrl + "/iframe_api";
inline const constexpr char youtubeBaseJs[] = "https://www.youtube.com/s/player/{}/player_ias.vflset/en_US/base.js";
inline const std::string swJsDataUrl = youtubeMainUrl + "/sw.js_data?hl=en";
inline const std::string youtubeBrowseUrl = youtubeMainUrl + "/youtubei/v1/browse";
inline const std::string youtubePlayerApiKey = "AIzaSyA8eiZmM1FaDVjRy-df2KTyQ_vz_yYM39w";

constexpr char youtubePostContent[] = R"delimiter(       
    {
        "videoId": null,
        "contentCheckOk": true,
        "context": {
            "client": {
                "clientName": "ANDROID_VR",
                "clientVersion": "1.60.19",
                "deviceMake": "Oculus",
                "deviceModel": "Quest 3",
                "platform": "MOBILE",
                "osName": "Android",
                "osVersion": "12L",
                "visitorData": null,
                "hl": "en",
                "gl": "US",
                "utcOffsetMinutes": 0
            }
        }
    })delimiter";

constexpr char youtubeBrowsePostContent[] = R"delimiter(       
    {
        "browseId": null,
        "context": {
            "client": {
                "clientName": "WEB",
                "clientVersion": "2.20250923.08.00",
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
inline const std::string youtube_visitor_user_agent = "User-Agent: com.google.android.youtube/20.10.38 (Linux; U; ANDROID 11) gzip";
inline const std::string youtube_player_user_agent =
    "User-Agent: com.google.android.apps.youtube.vr.oculus/1.60.19 (Linux; U; Android 12L; Quest 3 Build/SQ3A.220605.009.A1) gzip";
inline const std::string youtube_default_cookies = "SOCS=CAISEwgDEgk4MTM4MzYzNTIaAmVuIAEaBgiApPzGBg; domain=.youtube.com";
}  // namespace youtubeapi
