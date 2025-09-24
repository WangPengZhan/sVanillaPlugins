#pragma once
#include <string>

namespace weiboapi
{
constexpr char domain[] = ".weibo.com";

inline const std::string weiboHomeUrl = "https://weibo.com/";
inline const std::string weiboQrcodeUrl = "https://login.sina.com.cn/sso/qrcode/image";
inline const std::string weiboCheckUrl = "https://login.sina.com.cn/sso/qrcode/check";
inline const std::string weiboLoginUrl = "http://login.sina.com.cn/sso/login.php";
inline const std::string weiboDetailUrl = "https://m.weibo.cn/statuses/show?id=";
inline const std::string weiboVideoInfoUrl = "https://weibo.com/tv/api/component?page=";
inline const std::string weiboAjaxUrl = "https://weibo.com/ajax/statuses/show?id=";
inline const std::string weiboUserMblogUrl = "https://m.weibo.cn/api/container/getIndex?type=uid";
inline const std::string weiboMidRefererUrl = "https://weibo.com/tv/show/";
inline const std::string weiboUserInfoUrl = "https://weibo.com/ajax/profile/info?uid=";

inline const std::string weiboUserAgent =
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/90.0.4430.212 Safari/537.36";
inline const std::string weiboReferer = "https://mail.sina.com.cn/";

constexpr char weiboPostContent[] = R"delimiter(       
    {
        "Component_Play_Playinfo": 
        {
            "oid": ""
        }
    })delimiter";
}  // namespace weiboapi
