#pragma once

#include <nlohmann/json.hpp>
#include <list>

namespace biliapi
{
// BiliApi文档
// https://github.com/SocialSisterYi/bilibili-API-collect

class VideoOwner
{
public:
    long long mid;
    std::string name;
    std::string face;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(VideoOwner, mid, name, face)
};

class ArgueInfo
{
public:
    std::string argue_link;
    std::string argue_msg;
    int argue_type;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ArgueInfo, argue_link, argue_msg, argue_type)
};

class VideoStat
{
public:
    long long aid;
    long long view;
    long long danmaku;
    long long favorite;
    long long coin;
    long long share;
    long long now_rank;
    long long his_rank;
    long long like;
    long long dislike;
    std::string evaluation;
    ArgueInfo argue_info;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(VideoStat, aid, view, danmaku, favorite, coin, share, now_rank, his_rank, like, dislike, evaluation, argue_info)
};

class Dimension
{
public:
    int width;
    int height;
    int rotate;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Dimension, width, height, rotate)
};

class UgcArc
{
public:
    long long aid;
    std::string pic;
    std::string title;
    long long pubdate;
    std::string desc;
    int duration;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UgcArc, aid, pic, title, pubdate, desc, duration)
};

class VideoPage
{
public:
    long long cid;
    // int page;
    std::string first_frame;
    std::string part;
    int duration;
    Dimension dimension;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(VideoPage, cid, first_frame, part, duration, dimension)
};

class SubtitleAuthor
{
public:
    long long mid;
    std::string name;
    std::string sex;
    std::string face;
    std::string sign;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SubtitleAuthor, mid, name, sex, face, sign)
};

class Subtitle
{
public:
    long long id;
    std::string lan;
    std::string author_doc;
    bool is_lock;
    long long author_mid;
    std::string subtitle_url;
    SubtitleAuthor author;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Subtitle, id, lan, author_doc, is_lock, author_mid, subtitle_url, author)
};

class VideoSubtitle
{
public:
    bool allow_submit;
    std::list<Subtitle> list;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(VideoSubtitle, allow_submit, list)
};

class UgcStat
{
public:
    long long season_id;
    long long view;
    long long danmaku;
    long long relay;
    long long favorite;
    long long coin;
    long long share;
    long long nowRank;
    long long hisRank;
    long long like;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UgcStat, season_id, view, danmaku, relay, favorite, coin, share, nowRank, hisRank, like)
};

class UgcEpisode
{
public:
    long long aid;
    long long cid;
    std::string title;
    UgcArc arc;
    VideoPage page;
    std::string bvid;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UgcEpisode, aid, cid, title, arc, page, bvid)
};

class UgcSection
{
public:
    std::string title;
    std::list<UgcEpisode> episodes;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UgcSection, title, episodes)
};

class UgcSeason
{
public:
    long long id;
    std::string title;
    std::string intro;
    std::list<UgcSection> sections;
    int ep_count;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UgcSeason, id, title, intro, sections, ep_count)
};

class VideoView
{
public:
    std::string bvid;
    long long aid;
    int tid;
    std::string tname;
    std::string pic;
    std::string title;
    long long pubdate;
    std::string desc;
    int duration;
    VideoOwner owner;
    std::string dynamic;
    long long cid;
    std::list<VideoPage> pages;
    UgcSeason ugc_season;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(VideoView, bvid, aid, tid, tname, pic, title, pubdate, desc, duration, owner, dynamic, cid, pages, ugc_season)
};

class VideoViewOrigin
{
public:
    VideoView data;
    int code{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(VideoViewOrigin, data, code)
};

class SubtitleInfo
{
public:
    bool allow_submit;
    std::string lan;
    std::string lan_doc;
    std::list<Subtitle> subtitles;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SubtitleInfo, allow_submit, lan, lan_doc, subtitles)
};

class PlayerV2
{
public:
    long long aid;
    std::string bvid;
    long long cid;
    SubtitleInfo subtitle;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlayerV2, aid, bvid, cid, subtitle)
};

class PlayUrlDurl
{
public:
    int order;
    std::string length;
    long long size;
    std::string url;
    std::list<std::string> backup_url;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlayUrlDurl, order, length, size, backup_url)
};

class PlayUrlDashVideo
{
public:
    long long id;
    std::string base_url;
    std::list<std::string> backup_url;
    std::string mimeType;
    std::string codecs;
    int width;
    int height;
    std::string frameRate;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlayUrlDashVideo, id, base_url, backup_url, mimeType, codecs, width, height, frameRate)
};

class PlayUrlDashDolby
{
public:
    std::list<PlayUrlDashVideo> audio;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlayUrlDashDolby, audio)
};

class PlayUrlDash
{
public:
    std::list<PlayUrlDashVideo> video;
    std::list<PlayUrlDashVideo> audio;
    PlayUrlDashDolby dolby;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlayUrlDash, video, audio, dolby)
};

class SupportFormat
{
public:
    int quality;
    std::string format;
    std::string new_description;
    std::string description;
    std::string display_desc;
    std::string superscript;
    std::vector<std::string> codecs;
    bool need_login{};
    bool need_vip{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SupportFormat, quality, format, new_description, description, superscript, display_desc, codecs, need_vip)
};

class PlayUrl
{
public:
    std::list<std::string> accept_description;
    std::list<int> accept_quality;
    std::list<PlayUrlDurl> durl;
    PlayUrlDash dash;
    std::string accept_format;
    std::list<SupportFormat> support_formats;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlayUrl, accept_description, accept_quality, durl, dash, accept_format, support_formats)
};

class Durl
{
public:
    int order;
    int length;
    int size;
    std::string ahead;
    std::string vhead;
    std::string url;
    std::list<std::string> backup_url;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Durl, order, length, size, ahead, vhead, url, backup_url)
};

class DashSegmentBase
{
public:
    std::string initialization;
    std::string index_range;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(DashSegmentBase, initialization, index_range)
};

class DashInfo
{
public:
    int id;
    std::string baseUrl;
    std::string base_url;
    std::list<std::string> backupUrl;
    std::list<std::string> backup_url;
    int bandwidth;
    std::string mimeType;
    std::string mime_type;
    std::string codecs;
    int width;
    int height;
    std::string frameRate;
    std::string frame_rate;
    std::string sar;
    int startWithSap;
    int start_with_sap;
    DashSegmentBase SegmentBase;
    DashSegmentBase segment_base;
    int codecid;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(DashInfo, id, baseUrl, base_url, backupUrl, backup_url, bandwidth, mimeType, mime_type, codecs, width, height,
                                                frameRate, frame_rate, startWithSap, start_with_sap, SegmentBase, segment_base)
};

class DashDobly
{
public:
    int type;
    std::list<DashInfo> audio;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(DashDobly, type, audio)
};

class DashFlac
{
public:
    bool display;
    std::list<DashInfo> audio;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(DashFlac, display, audio)
};

class PlayDash
{
public:
    int duration;
    int minBufferTime;
    int min_buffer_time;
    std::list<DashInfo> video;
    std::list<DashInfo> audio;
    DashDobly dolby;
    DashFlac flac;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlayDash, duration, minBufferTime, min_buffer_time, video, audio)
};

class PlayUrlData
{
public:
    std::string from;
    std::string result;
    std::string message;
    int quality;
    std::string format;
    int timelength;
    std::string accept_format;
    std::list<std::string> accept_description;
    std::list<int> accept_quality;
    int video_codecid;
    std::string seek_param;
    std::string seek_type;
    std::list<Durl> durl;
    std::list<SupportFormat> support_formats;
    PlayDash dash;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlayUrlData, from, result, message, quality, format, timelength, accept_format, accept_description,
                                                accept_quality, video_codecid, seek_param, seek_type, durl, support_formats, dash)
};

class PlayUrlOrigin
{
public:
    PlayUrlData data;
    int code{};
    int ttl;
    std::string message;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlayUrlOrigin, data, code, ttl, message)
};

class LoginUrl
{
public:
    std::string qrcode_key;
    std::string url;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LoginUrl, qrcode_key, url)
};

class LoginUrlOrigin
{
public:
    LoginUrl data;
    int code;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LoginUrlOrigin, data, code)
};

class LoginStatusData
{
public:
    std::string url;
    std::string refresh_token;
    std::string message;
    int code;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LoginStatusData, url, refresh_token, message, code)
};

class LoginStatusScanning
{
public:
    LoginStatusData data;
    int code{};
    std::string message;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LoginStatusScanning, data, code, message)
};

class WbiImg
{
public:
    std::string img_url;
    std::string sub_url;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WbiImg, img_url, sub_url)
};

class NavData
{
public:
    bool isLogin;
    int email_verified;
    std::string face;
    int mid;
    int mobile_verified;
    int money;
    int moral;
    std::string uname;
    int vipType;
    WbiImg wbi_img;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NavData, isLogin, email_verified, face, mid, mobile_verified, money, moral, uname, vipType, wbi_img)
};

class Nav
{
public:
    NavData data;
    int code{};
    std::string message;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Nav, data, code, message)
};

class LoginStatus
{
public:
    int code;
    bool status;
    std::string message;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LoginStatus, code, status, message)
};

class LoginStatusReady
{
public:
    int code;
    LoginStatusData data;
    bool status;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LoginStatusReady, code, status, data)
};

class RedirectUrl
{
public:
    std::string redirectUrl;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(RedirectUrl, redirectUrl)
};

class LogoutExitV2
{
public:
    int code;
    bool status;
    uint64_t ts;
    std::string message;
    RedirectUrl data;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LogoutExitV2, code, status, ts, message, data)
};

class MixinKey
{
public:
    std::time_t Expires;
    std::string mixin_key;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MixinKey, Expires, mixin_key)
};

class HistoryCursor
{
public:
    int max;
    int view_at;
    std::string business;
    int ps;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(HistoryCursor, max, view_at, business, ps)
};

class HistoryTab
{
public:
    std::string type;
    std::string name;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(HistoryTab, type, name)
};

class HistoryDetailInfo
{
public:
    long long oid;
    long long epid;
    std::string bvid;
    int page;
    long long cid;
    std::string part;
    std::string business;
    int dt;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(HistoryDetailInfo, oid, epid, bvid, page, cid, part, business, dt)
};

class HistoryInfo
{
public:
    std::string title;
    std::string long_title;
    std::string cover;
    std::vector<std::string> covers;
    std::string uri;
    HistoryDetailInfo history;
    int videos;
    std::string author_name;
    std::string author_face;
    int author_mid;
    uint64_t view_at;
    int progress;
    std::string badge;
    int duration;
    int total;
    std::string new_desc;
    int is_finish;
    int is_fav;
    int kid;
    std::string tag_name;
    int live_status;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(HistoryInfo, title, long_title, cover, uri, history, videos, author_name, author_face, author_mid, view_at,
                                                progress, badge, duration, total, new_desc, is_finish, is_fav, kid, tag_name, live_status)
};

class HistoryQueryInfo
{
public:
    HistoryCursor cursor;
    std::vector<HistoryTab> tab;
    std::vector<HistoryInfo> list;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(HistoryQueryInfo, cursor, tab, list)
};

class History
{
public:
    int code;
    std::string message;
    int ttl;
    HistoryQueryInfo data;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(History, code, message, ttl, data)
};

template <typename T>
struct ResultResponse
{
    int code;
    std::string message;
    T result;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ResultResponse<T>, code, message, result)
};

struct BadgeInfo
{
    std::string bg_color;
    std::string bg_color_night;
    std::string text;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(BadgeInfo, bg_color, bg_color_night, text)
};

struct IconFont
{
    std::string name;
    std::string text;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(IconFont, name, text)
};

struct Interaction
{
    int graph_version{};
    bool interaction{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Interaction, graph_version, interaction)
};

struct Rights
{
    int allow_demand{};
    int allow_dm{};
    int allow_download{};
    int area_limit{};
    std::string copyright;
    int forbid_pre{};
    int freya_white{};
    int is_cover_show{};
    int is_preview{};
    int only_vip_download{};
    std::string resource;
    int watch_platform{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Rights, allow_demand, allow_dm, allow_download, area_limit, copyright, forbid_pre, freya_white, is_cover_show,
                                                is_preview, only_vip_download, resource, watch_platform)
};

struct SectionTime
{
    int64_t start{};
    int64_t end{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SectionTime, start, end)
};

struct Skip
{
    SectionTime ed;
    SectionTime op;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Skip, ed, op)
};

struct Episode
{
    int64_t aid{};
    int64_t archive_attr{};
    std::string badge;
    BadgeInfo badge_info;
    int badge_type{};
    std::string bvid;
    int64_t cid{};
    std::string cover;
    Dimension dimension;
    int64_t duration{};
    bool enable_vt{};
    int64_t ep_id{};
    std::string from;
    int64_t id{};
    Interaction interaction;
    bool is_sub_view{};
    bool is_view_hide{};
    std::string link;
    std::string link_type;
    std::string long_title;
    std::vector<int64_t> multi_view_eps;
    int64_t pub_time{};
    int pv{};
    std::string release_date;
    Rights rights;
    int section_type{};
    std::string share_copy;
    std::string share_url;
    std::string short_link;
    std::string show_title;
    bool showDrmLoginDialog{};
    Skip skip;
    int status{};
    std::string subtitle;
    std::string title;
    std::string toast_title;
    std::string vid;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Episode, aid, archive_attr, badge, badge_info, badge_type, bvid, cid, cover, dimension, duration, enable_vt,
                                                ep_id, from, id, interaction, is_sub_view, is_view_hide, link, link_type, long_title, multi_view_eps, pub_time,
                                                pv, release_date, rights, section_type, share_copy, share_url, short_link, show_title, showDrmLoginDialog,
                                                status, subtitle, title, toast_title, vid)
};

struct NewEpisode
{
    std::string desc;
    int64_t id{};
    int is_new{};
    std::string title;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NewEpisode, desc, id, is_new, title)
};

struct Pendant
{
    std::string image;
    std::string name;
    int pid;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Pendant, image, name, pid)
};

struct Activity
{
    std::string cover;
    int id;
    std::string link;
    std::vector<Pendant> pendants;
    std::string title;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Activity, cover, id, link, pendants, title)
};

struct Area
{
    int id;
    std::string name;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Area, id, name)
};

struct Freya
{
    std::string bubble_desc;
    int64_t bubble_show_cnt;
    int64_t icon_show;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Freya, bubble_desc, bubble_show_cnt, icon_show)
};

struct MultiViewInfo
{
    std::string changing_dance;
    bool is_multi_view_season{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MultiViewInfo, changing_dance, is_multi_view_season)
};

struct PayType
{
    int allow_discount{};
    int allow_pack{};
    int allow_ticket{};
    int allow_time_limit{};
    int allow_vip_discount{};
    int forbid_bb{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PayType, allow_discount, allow_pack, allow_ticket, allow_time_limit, allow_vip_discount, forbid_bb)
};

struct Payment
{
    int discount{};
    PayType pay_type;
    std::string price;
    std::string promotion;
    std::string tip;
    int64_t view_start_time{};
    int vip_discount{};
    std::string vip_first_promotion;
    std::string vip_price;
    std::string vip_promotion;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Payment, discount, pay_type, price, promotion, tip, view_start_time, vip_discount, vip_first_promotion,
                                                vip_price, vip_promotion)
};

struct PayPack
{
    int64_t id{};
    std::string not_paid_text_for_app;
    std::string paid_text_for_app;
    std::string pay_pack_url;
    int status{};
    std::string title;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PayPack, id, not_paid_text_for_app, paid_text_for_app, pay_pack_url, status, title)
};

struct Positive
{
    int64_t id{};
    std::string title;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Positive, id, title)
};

struct Publish
{
    int is_finish{};
    int is_started{};
    std::string pub_time;
    std::string pub_time_show;
    int unknow_pub_date{};
    int weekday{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Publish, is_finish, is_started, pub_time, pub_time_show, unknow_pub_date, weekday)
};

struct Rating
{
    int64_t count{};
    double score{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Rating, count, score)
};

struct Stat
{
    int64_t favorites{};
    int64_t series_follow{};
    int64_t views{};
    int64_t vt{};
    int64_t play{};
    std::string play_desc;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Stat, favorites, series_follow, views, vt, play, play_desc)
};

struct Season
{
    std::string badge;
    BadgeInfo badge_info;
    int badge_type{};
    std::string cover;
    bool enable_vt{};
    std::string horizontal_cover_1610;
    std::string horizontal_cover_169;
    IconFont icon_font;
    int64_t media_id{};
    NewEpisode new_ep;
    int64_t season_id{};
    std::string season_title;
    int season_type{};
    Stat stat;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Season, badge, badge_info, badge_type, cover, enable_vt, horizontal_cover_1610, horizontal_cover_169, icon_font,
                                                media_id, new_ep, season_id, season_title, season_type, stat)
};

struct Report
{
    std::string aid;
    std::string ep_title;
    std::string position;
    std::string season_id;
    std::string season_type;
    std::string section_id;
    std::string section_type;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Report, aid, ep_title, position, season_id, season_type, section_id, section_type)
};

struct Section
{
    int attr{};
    int episode_id{};
    std::vector<int64_t> episode_ids;
    std::vector<Episode> episodes;
    int64_t id{};
    Report report;
    std::string title;
    int type{};
    int type2{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Section, attr, episode_id, episode_ids, episodes, id, report, title, type, type2)
};

struct Series
{
    int display_type{};
    int64_t series_id{};
    std::string series_title;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Series, display_type, series_id, series_title)
};

struct UpInfoPendant
{
    std::string image;
    std::string name;
    int64_t pid{};
    int64_t follower{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UpInfoPendant, image, name, pid, follower)
};

struct UpInfoVipLabel
{
    std::string bg_color;
    int bg_style{};
    std::string border_color;
    std::string text;
    std::string text_color;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UpInfoVipLabel, bg_color, bg_style, border_color, text, text_color)
};

struct UpInfo
{
    std::string avatar;
    std::string avatar_subscript_url;
    std::string brief;
    int64_t follower{};
    int is_follow{};
    std::string link;
    int64_t mid{};
    std::string nickname_color;
    UpInfoPendant pendant;
    int theme_type{};
    std::string uname;
    int verify_type{};
    UpInfoVipLabel vip_label;
    int vip_status{};
    int vip_type{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UpInfo, avatar, avatar_subscript_url, brief, follower, is_follow, link, mid, nickname_color, pendant,
                                                theme_type, uname, verify_type, vip_label, vip_status, vip_type)
};

struct UserStatus
{
    int area_limit{};
    int ban_area_show{};
    int follow{};
    int follow_status{};
    int login{};
    int pay{};
    int pay_pack_paid{};
    int sponsor{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UserStatus, area_limit, ban_area_show, follow, follow_status, login, pay, pay_pack_paid, sponsor)
};

struct BangumiData
{
    Activity activity;
    std::string actors;
    std::string alias;
    std::vector<Area> areas;
    std::string bkg_cover;
    std::string cover;
    bool delivery_fragment_video{};
    bool enable_vt{};
    std::vector<Episode> episodes;
    std::string evaluate;
    Freya freya;
    int hide_ep_vv_vt_dm{};
    IconFont icon_font;
    std::string jp_title;
    std::string link;
    int64_t media_id{};
    int mode{};
    MultiViewInfo multi_view_info;
    NewEpisode new_ep;
    Payment payment;
    PayPack payPack;
    Positive positive;
    Publish publish;
    Rating rating;
    std::string record;
    Rights rights;
    int64_t season_id{};
    std::string season_title;
    std::vector<Season> seasons;
    std::vector<Section> section;
    std::string section_bottom_desc;
    Series series;
    std::string share_copy;
    std::string share_sub_title;
    std::string share_url;
    int show_season_type{};
    std::string square_cover;
    std::string staff;
    Stat stat;
    int status{};
    std::vector<std::string> styles;
    std::string subtitle;
    std::string title;
    int total{};
    int type{};
    UpInfo up_info;
    UserStatus user_status;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(BangumiData, activity, actors, alias, areas, bkg_cover, cover, delivery_fragment_video, enable_vt, episodes,
                                                evaluate, freya, hide_ep_vv_vt_dm, icon_font, jp_title, link, media_id, mode, multi_view_info, new_ep, payment,
                                                payPack, positive, publish, rating, record, rights, season_id, season_title, seasons, section,
                                                section_bottom_desc, series, share_copy, share_sub_title, share_url, show_season_type, square_cover, staff,
                                                stat, status, styles, subtitle, title, total, type, up_info, user_status)
};

using BangumiInfo = ResultResponse<BangumiData>;

struct MediaNewEp
{
    int64_t id{};
    std::string index;
    std::string index_show;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MediaNewEp, id, index, index_show)
};

struct MediaBaseInfo
{
    std::vector<Area> areas;
    std::string cover;
    std::string horizontal_picture;
    int64_t media_id{};
    MediaNewEp new_ep;
    Rating rating;
    int64_t season_id{};
    std::string share_url;
    std::string title;
    int type{};
    std::string type_name;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MediaBaseInfo, areas, cover, horizontal_picture, media_id, new_ep, rating, season_id, share_url, title, type,
                                                type_name)
};

struct Review
{
    int is_coin{};
    int is_open{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Review, is_coin, is_open)
};

struct MediaInfo
{
    MediaBaseInfo media;
    Review review;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MediaInfo, media, review)
};

using MediaInfoResponse = ResultResponse<MediaInfo>;

template <typename T>
struct DataResponse
{
    int code{-1};
    std::string message;
    T data;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(DataResponse<T>, code, message, data)
};

struct BriefImage
{
    int aspect_ratio{};
    std::string url;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(BriefImage, aspect_ratio, url)
};

struct Brief
{
    std::string content;
    std::vector<BriefImage> img;
    std::string title;
    int type{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Brief, content, img, title, type)
};

struct Coupon
{
    int amount{};
    std::string expire_time;
    std::string start_time;
    int status{};
    std::string title;
    std::string token;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Coupon, amount, expire_time, start_time, status, title, token)
};

struct EpisodePage
{
    bool next{};
    int num{};
    int size{};
    int total{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(EpisodePage, next, num, size, total)
};

struct CheeseEpisode
{
    int aid{};
    int cid{};
    std::string cover;
    int duration{};
    int ep_status{};
    bool episode_can_view{};
    std::string from;
    int id{};
    int index{};
    int page{};
    int play{};
    int pay_way{};
    int playable;
    int64_t release_date{};
    int status{};
    std::string subtitle;
    std::string title;
    bool watched{};
    int watchedHistory{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CheeseEpisode, aid, cid, cover, duration, ep_status, episode_can_view, from, id, index, page, play, pay_way,
                                                playable, release_date, status, subtitle, title, watched, watchedHistory)
};

struct FAQ
{
    std::string content;
    std::string link;
    std::string title;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FAQ, content, link, title)
};

struct FAQ1Item
{
    std::string answer;
    std::string question;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FAQ1Item, answer, question)
};

struct FAQ1
{
    std::vector<FAQ1Item> items;
    std::string title;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FAQ1, items, title)
};

struct CheesePayment
{
    std::string desc;
    std::string discount_desc;
    std::string discount_prefix;
    std::string pay_shade;
    int price{};
    std::string price_format;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CheesePayment, desc, discount_desc, discount_prefix, pay_shade, price, price_format)
};

struct PurchaseNote
{
    std::string content;
    std::string link;
    std::string title;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PurchaseNote, content, link, title)
};

struct PurchaseProtocol
{
    std::string link;
    std::string title;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PurchaseProtocol, link, title)
};

struct Progress
{
    int last_ep_id{};
    std::string last_ep_index;
    int last_time{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Progress, last_ep_id, last_ep_index, last_time)
};

struct UserStatusInfo
{
    int favored{};
    int favored_count{};
    int payed{};
    Progress progress;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UserStatusInfo, favored, favored_count, payed, progress)
};

struct CheeseInfo
{
    Brief brief;
    Coupon coupon;
    std::string cover;
    EpisodePage episode_page;
    int episode_sort{};
    std::vector<CheeseEpisode> episodes;
    FAQ faq;
    FAQ1 faq1;
    CheesePayment payment;
    PurchaseNote purchase_note;
    PurchaseProtocol purchase_protocol;
    std::string release_bottom_info;
    std::string release_info;
    std::string release_info2;
    std::string release_status;
    int season_id{};
    std::string share_url;
    std::string short_link;
    Stat stat;
    int status{};
    std::string subtitle;
    std::string title;
    UpInfo up_info;
    UserStatusInfo user_status;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CheeseInfo, brief, coupon, cover, episode_page, episode_sort, episodes, faq, faq1, payment, purchase_note,
                                                purchase_protocol, release_bottom_info, release_info, release_info2, release_status, season_id, share_url,
                                                short_link, stat, status, subtitle, title, up_info, user_status)
};

using CheeseInfoResponse = DataResponse<CheeseInfo>;

struct FavUpper
{
    int64_t mid{};
    std::string name;
    std::string face;
    bool followed{};
    int vip_type{};
    int vip_statue{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FavUpper, mid, name, face, followed, vip_type, vip_statue)
};

struct FavCntInfo
{
    int collect{};
    int play{};
    int thumb_up{};
    int share{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FavCntInfo, collect, play, thumb_up, share)
};

struct FavData
{
    int64_t id{};
    int64_t fid{};
    int64_t mid{};
    int attr{};
    std::string title;
    std::string cover;
    FavUpper upper;
    int cover_type{};
    FavCntInfo cnt_info;
    int type{};
    std::string intro;
    int64_t ctime{};
    int64_t mtime{};
    int state{};
    int fav_state{};
    int like_state{};
    int media_count{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FavData, id, fid, mid, attr, title, cover, upper, cover_type, cnt_info, type, intro, ctime, mtime, state,
                                                fav_state, like_state, media_count)
};

using FavDataResponse = DataResponse<FavData>;

struct FavListInfo
{
    int count;
    std::vector<FavData> list;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FavListInfo, count, list)
};

using CreatedFavResponse = DataResponse<FavListInfo>;

struct FavItemInfo
{
    int64_t id;
    int type;
    std::string bv_id;
    std::string bvid;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FavItemInfo, id, type, bv_id, bvid)
};

using FavDetailInfo = DataResponse<std::vector<FavItemInfo>>;

struct UpperInfo
{
    int64_t mid = 0;
    std::string name;
    std::string face;
    std::string jump_link;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UpperInfo, mid, name, face, jump_link)
};

struct CountInfo
{
    int collect = 0;
    int64_t play = 0;
    int danmaku = 0;
    int vt = 0;
    int play_switch = 0;
    int reply = 0;
    std::string view_text_1;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CountInfo, collect, play, danmaku, vt, play_switch, reply, view_text_1)
};

struct UGCInfo
{
    int64_t first_cid = 0;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UGCInfo, first_cid)
};

struct FavVideoInfo
{
    int64_t id{};
    int type{};
    std::string title;
    std::string cover;
    std::string intro;
    int page{};
    int duration{};
    UpperInfo upper;
    int attr{};
    CountInfo cnt_info;
    std::string link;
    int64_t ctime{};
    int64_t pubtime{};
    int64_t fav_time{};
    std::string bv_id;
    std::string bvid;
    UGCInfo ugc;
    std::string media_list_link;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FavVideoInfo, id, type, title, cover, intro, page, duration, upper, attr, cnt_info, link, ctime, pubtime,
                                                fav_time, bv_id, bvid, ugc, media_list_link)
};

using FavVideoInfoResponse = DataResponse<std::vector<FavVideoInfo>>;

struct RecordInfo
{
    std::string record_icon;
    std::string record;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(RecordInfo, record_icon, record)
};

struct UgcPlayUrl
{
    int code{};
    int is_preview{};
    int fnver{};
    int fnval{};
    bool video_project{};
    std::string type;
    int bp{};
    int vip_type{};
    int vip_status{};
    bool is_drm{};
    int no_rexcode{};
    bool has_paid{};
    int status{};
    std::string from;
    std::string result;
    std::string message;
    int quality{};
    std::string format;
    int64_t timelength{};
    std::string accept_format;
    std::vector<std::string> accept_description;
    std::vector<int> accept_quality;
    int video_codecid{};
    std::string seek_param;
    std::string seek_type;

    std::vector<PlayUrlDurl> durl;
    PlayDash dash;

    std::vector<SupportFormat> support_formats;
    RecordInfo record_info;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UgcPlayUrl, code, is_preview, fnver, fnval, video_project, type, bp, vip_type, vip_status, is_drm, no_rexcode,
                                                has_paid, status, from, result, message, quality, format, timelength, accept_format, accept_description,
                                                accept_quality, video_codecid, seek_param, seek_type, durl, dash, support_formats, record_info)
};

using UgcPlayUrlResponse = ResultResponse<UgcPlayUrl>;

struct CheesePlayUrl
{
    std::string accept_format;
    int code{};
    std::vector<PlayUrlDurl> durl;
    PlayDash dash;
    std::string seek_param;
    int no_rexcode{};
    std::string format;
    int fnval{};
    bool video_project{};
    int fnver{};
    std::string message;
    std::string type;
    std::vector<int> accept_quality;
    int quality{};
    size_t timelength{};
    std::string result;
    std::string seek_type;
    bool has_paid{};
    std::vector<SupportFormat> supportFormats;
    std::string from;
    int video_codecid{};
    std::vector<std::string> accept_description;
    int status{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CheesePlayUrl, accept_format, code, durl, dash, seek_param, no_rexcode, format, fnval, video_project, fnver,
                                                message, type, accept_quality, quality, timelength, result, seek_type, has_paid, supportFormats, from,
                                                video_codecid, accept_description, status)
};

using CheesePlayUrlResponse = DataResponse<CheesePlayUrl>;

struct MetaStat
{
    int coin{};
    int danmaku{};
    int favorite{};
    int like{};
    long long mtime{};
    int reply{};
    int season_id{};
    int share{};
    int view{};
    int vt{};
    int vv{};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MetaStat, coin, danmaku, favorite, like, mtime, reply, season_id, share, view, vt, vv)
};
struct Meta
{
    int attribute{};
    std::string cover;
    int ep_count{};
    int ep_num{};
    int first_aid{};
    int id{};
    std::string intro;
    long long mid{};
    long long ptime{};
    int sign_state{};
    MetaStat stat;
    std::string title;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Meta, attribute, cover, ep_count, ep_num, first_aid, id, intro, mid, ptime, sign_state, stat, title)
};
struct VlistItem
{
    long long aid{};
    int attribute{};
    std::string author;
    std::string bvid;
    int comment{};
    std::string copyright;
    long long created{};
    std::string description;
    int elec_arc_type{};
    int enable_vt{};
    bool hide_click{};
    int is_avoided{};
    bool is_charging_arc{};
    int is_lesson_video{};
    int is_lesson_finished{};
    int is_live_playback{};
    int is_pay{};
    bool is_self_view{};
    int is_steins_gate{};
    int is_union_video{};
    std::string jump_url;
    std::string length;
    long long mid{};
    Meta meta;
    std::string pic;
    long long play{};
    int playback_position{};
    int review{};
    int season_id{};
    std::string subtitle;
    std::string title;
    int typeid_;
    int video_review{};
    int vt{};
    std::string vt_display;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(VlistItem, aid, attribute, author, bvid, comment, copyright, created, description, elec_arc_type, enable_vt,
                                                hide_click, is_avoided, is_charging_arc, is_lesson_video, is_lesson_finished, is_live_playback, is_pay,
                                                is_self_view, is_steins_gate, is_union_video, jump_url, length, mid, meta, pic, play, playback_position, review,
                                                season_id, subtitle, title, typeid_, video_review, vt, vt_display)
};
struct TlistItem
{
    int count{};
    std::string name;
    int tid{};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(TlistItem, count, name, tid)
};
struct VideoList
{
    std::map<std::string, TlistItem> tlist;
    std::vector<VlistItem> vlist;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(VideoList, tlist, vlist)
};
struct PageInfo
{
    int count{};
    int pn{};
    int ps{};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PageInfo, count, pn, ps)
};
struct EpisodicButton
{
    std::string text;
    std::string uri;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(EpisodicButton, text, uri)
};
struct VideoWorks
{
    VideoList list;
    PageInfo page;
    EpisodicButton episodic_button;
    bool is_risk{};
    int gaia_res_type{};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(VideoWorks, list, page, episodic_button, is_risk, gaia_res_type)
};

using VideoWorksResponse = DataResponse<VideoWorks>;

struct BiliTicket
{
    std::string ticket;
    int64_t created_at{};
    int64_t ttl{};
    WbiImg nav;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(BiliTicket, ticket, created_at, ttl, nav)
};

using BiliTicketResponse = DataResponse<BiliTicket>;

struct BuvidInfo
{
    std::string buvid;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(BuvidInfo, buvid)
};

struct Buvid34Info
{
    std::string b_3;
    std::string b_4;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Buvid34Info, b_3, b_4)
};

using BuvidInfoResponse = DataResponse<BuvidInfo>;
using Buvid34InfoResponse = DataResponse<Buvid34Info>;

}  // namespace biliapi
