#pragma once
#include <string>

namespace xhsapi
{

inline const std::string apiBase = "https://edith.xiaohongshu.com";
inline const std::string home = "https://www.xiaohongshu.com";
inline const std::string asBase = "https://as.xiaohongshu.com";
constexpr char domain[] = "xiaohongshu.com";

namespace Login
{
inline const std::string QRCreate = "/api/sns/web/v1/login/qrcode/create";
inline const std::string QRCheck = "/api/qrcode/userinfo";
inline const std::string AccountInfo = "/api/sns/web/v2/user/me";
inline const std::string Activate = "/api/sns/web/v1/login/activate";

}  // namespace Login

namespace Api
{
inline const std::string HomeFeed = "/api/sns/web/v1/homefeed";
inline const std::string NoteDetail = "/api/sns/web/v1/feed";
inline const std::string NoteComments = "/api/sns/v1/comment/list";
inline const std::string UserInfo = "/api/sns/v1/user/profile";
inline const std::string UserNotes = "/api/sns/web/v1/user/notes";
inline const std::string SearchNotes = "/api/sns/v1/search/notes";
inline const std::string UserAllNotes = "/api/sns/web/v1/user_posted";
inline const std::string FavoriteNotes = "/api/sns/web/v2/note/collect/page";
inline const std::string LikeNotes = "/api/sns/web/v1/note/like/page";

inline const std::string Explore = "/explore";

inline const std::string Scripting = "/api/sec/v1/scripting";
inline const std::string WebProfile = "/api/sec/v1/sheild/profile";

}  // namespace Api

constexpr char scriptingData[] = R"delimiter({
    "appId": "xhs-pc-web",
    "callFrom": "web",
    "callback": "",
    "type": "ds
})delimiter";

namespace Encrypt
{
inline const std::string BASE62 = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
}

}  // namespace xhsapi
