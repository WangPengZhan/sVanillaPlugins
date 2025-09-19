#pragma once
#include <string>

namespace biliapi
{
constexpr char domain[] = "bilibili.com";

inline const std::string apiBase = "https://api.bilibili.com/";
inline const std::string passBase = "https://passport.bilibili.com/";

namespace PassportURL
{
inline const std::string QRCode = passBase + "x/passport-login/web/qrcode/generate";
inline const std::string LoginStatus = passBase + "x/passport-login/web/qrcode/poll";
inline const std::string WebNav = apiBase + "x/web-interface/nav";
inline const std::string Logout = passBase + "login/exit/v2";
inline const std::string History = apiBase + "x/web-interface/history/cursor";
}  // namespace PassportURL

inline const std::string ticketUrl = apiBase + "bapis/bilibili.api.ticket.v1.Ticket/GenWebTicket";
inline const std::string buvidUrl = apiBase + "x/web-frontend/getbuvid";
inline const std::string buvid34Url = apiBase + "x/frontend/finger/spi";

namespace VideoURL
{
inline const std::string View = apiBase + "x/web-interface/view";
inline const std::string Playurl = apiBase + "x/player/wbi/playurl";

inline const std::string PgcSeason = apiBase + "pgc/view/web/season";
inline const std::string MdSeason = apiBase + "pgc/review/user";
inline const std::string PgcPlayUrl = apiBase + "pgc/player/web/playurl";

inline const std::string CheeseSeason = apiBase + "pugv/view/web/season";
inline const std::string CheesePlayUrlApi = apiBase + "pugv/player/web/playurl";

inline const std::string FavInfoUrl = apiBase + "x/v3/fav/folder/info";
inline const std::string CreatedFavList = apiBase + "x/v3/fav/folder/created/list-all";
inline const std::string CollectedFavList = apiBase + "x/v3/fav/folder/collected/list";
inline const std::string FavInfoDetail = apiBase + "x/v3/fav/resource/ids";
inline const std::string FavVideoInfo = apiBase + "x/v3/fav/resource/infos";

inline const std::string UserVideoUrl = apiBase + "x/space/wbi/arc/search";
}  // namespace VideoURL

namespace Headers
{
inline const std::string DefaultReferer = "Referer: https://www.bilibili.com/";
inline const std::string PassportReferer = "Referer: https://passport.bilibili.com/";
}  // namespace Headers
}  // namespace biliapi
