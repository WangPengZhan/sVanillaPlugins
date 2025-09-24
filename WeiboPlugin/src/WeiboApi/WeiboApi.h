#pragma once

#include <nlohmann/json.hpp>
#include <list>
#include <iostream>

namespace weiboapi
{
struct WeiboQrcData
{
    std::string qrid;
    std::string image;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WeiboQrcData, qrid, image)
};

template <typename Data>
struct WeiboResponse
{
    int64_t retcode;
    std::string msg;
    Data data;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WeiboResponse<Data>, retcode, msg, data)
};

struct LoginScaningData
{
    std::string alt;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LoginScaningData, alt)
};

using QRCResponse = WeiboResponse<WeiboQrcData>;

using LoginScaningStatus = WeiboResponse<LoginScaningData>;

struct LoginInfo
{
    std::string retcode;
    std::string uid;
    std::string nick;
    std::vector<std::string> crossDomainUrlList;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LoginInfo, retcode, uid, nick, crossDomainUrlList)
};

struct CardlistInfo
{
    std::string containerid;
    int v_p;
    int show_style;
    int total;
    int autoLoadMoreIndex;
    long long since_id;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CardlistInfo, containerid, v_p, show_style, total, autoLoadMoreIndex, since_id)
};

struct User
{
    long long id{};
    std::string screen_name;
    std::string profile_image_url;
    std::string profile_url;
    bool close_blue_v{};
    std::string description;
    bool follow_me{};
    bool following{};
    int follow_count{};
    std::string cover_image_phone;
    std::string avatar_large;
    std::string avatar_hd;
    int statuses_count{};
    bool verified{};
    int verified_type{};
    std::string gender;
    std::string location;
    int mbtype{};
    int svip{};
    int urank{};
    int mbrank{};
    std::string followers_count_str;
    int verified_type_ext;
    std::string verified_reason;
    int user_type{};
    std::string is_star;
    bool like{};
    bool like_me{};
    bool is_muteuser{};
    bool special_follow{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(User, id, screen_name, profile_image_url, profile_url, close_blue_v, description, follow_me, following,
                                                follow_count, cover_image_phone, avatar_large, avatar_hd, statuses_count, verified, verified_type, gender,
                                                location, mbtype, svip, urank, mbrank, followers_count_str, verified_type_ext, verified_reason, user_type,
                                                is_star, like, like_me, is_muteuser, special_follow)
};

struct Geo
{
    int width;
    int height;
    bool croped;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Geo, width, height, croped)
};

struct Large
{
    std::string size;
    std::string url;
    Geo geo;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Large, size, url, geo)
};

struct Pic
{
    std::string pid;
    std::string url;
    std::string size;
    Geo geo;
    Large large;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Pic, pid, url, size, geo, large)
};

struct CommentManageInfo
{
    int comment_permission_type;
    int approval_comment_type;
    int comment_sort_type;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CommentManageInfo, comment_permission_type, approval_comment_type, comment_sort_type)
};

struct PagePic
{
    std::string width;
    std::string pid;
    std::string source;
    std::string is_self_cover;
    std::string type;
    std::string url;
    std::string height;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PagePic, width, pid, source, is_self_cover, type, url, height)
};

struct PageMediaInfo
{
    std::string stream_url;
    std::string stream_url_hd;
    double duration;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PageMediaInfo, stream_url, stream_url_hd, duration)
};

struct Urls
{
    std::string mp4_hd_mp4;
    std::string mp4_ld_mp4;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Urls, mp4_hd_mp4, mp4_ld_mp4)
};

struct PageInfo
{
    std::string type;
    int object_type;
    std::string url_ori;
    PagePic page_pic;
    std::string page_url;
    std::string object_id;
    std::string page_title;
    std::string title;
    std::string content1;
    std::string content2;
    std::string video_orientation;
    std::string play_count;
    PageMediaInfo media_info;
    Urls urls;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PageInfo, type, object_type, url_ori, page_pic, page_url, object_id, page_title, title, content1, content2,
                                                video_orientation, play_count, media_info, urls)
};

struct NumberDisplayStrategy
{
    int apply_scenario_flag;
    int display_text_min_number;
    std::string display_text;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NumberDisplayStrategy, apply_scenario_flag, display_text_min_number, display_text)
};

struct RetweetedStatus
{
    std::string created_at;
    std::string id;
    std::string mid;
    bool can_edit;
    std::string text;
    int textLength;
    std::string source;
    bool favorited;
    std::vector<std::string> pic_ids;
    bool is_paid;
    int mblog_vip_type;
    int reposts_count;
    int comments_count;
    int reprint_cmt_count;
    int attitudes_count;
    int mixed_count;
    int pending_approval_count;
    bool isLongText;
    int show_mlevel;
    bool ad_marked;
    int mblogtype;
    std::string item_category;
    std::string rid;
    std::string cardid;
    NumberDisplayStrategy number_display_strategy;
    int content_auth;
    bool is_show_mixed;
    int64_t safe_tags;
    CommentManageInfo comment_manage_info;
    int pic_num;
    int64_t fid;
    int mlevel;
    std::string region_name;
    int region_opt;
    int detail_bottom_bar;
    PageInfo page_info;
    User user;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(RetweetedStatus, created_at, id, mid, can_edit, text, textLength, source, favorited, pic_ids, is_paid,
                                                mblog_vip_type, reposts_count, comments_count, reprint_cmt_count, attitudes_count, mixed_count,
                                                pending_approval_count, isLongText, show_mlevel, ad_marked, mblogtype, item_category, rid, cardid,
                                                number_display_strategy, content_auth, is_show_mixed, safe_tags, comment_manage_info, pic_num, fid, mlevel,
                                                region_name, region_opt, detail_bottom_bar, page_info, user)
};

struct Mblog
{
    std::string mark;
    std::string created_at;
    std::string id;
    std::string mid;
    int edit_count;
    bool can_edit;
    std::string edit_at;
    std::string text;
    int textLength;
    std::string source;
    bool favorited;
    std::vector<std::string> pic_ids;
    std::string thumbnail_pic;
    std::string bmiddle_pic;
    std::string original_pic;
    bool is_paid;
    int mblog_vip_type;
    int reposts_count;
    int comments_count;
    int reprint_cmt_count;
    int attitudes_count;
    int mixed_count;
    int pending_approval_count;
    bool isLongText;
    int show_mlevel;
    long long expire_time;
    int ad_state;
    bool ad_marked;
    int mblogtype;
    std::string item_category;
    std::string rid;
    std::string mlevelSource;
    std::string cardid;
    std::string comment_guide_ext;
    int content_auth;
    bool is_show_mixed;
    long long safe_tags;
    CommentManageInfo comment_manage_info;
    int pic_num;
    int mlevel;
    std::vector<Pic> pics;
    std::string bid;
    User user;
    RetweetedStatus retweeted_status;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Mblog, mark, created_at, id, mid, edit_count, can_edit, edit_at, text, textLength, source, favorited, pic_ids,
                                                thumbnail_pic, bmiddle_pic, original_pic, is_paid, mblog_vip_type, reposts_count, comments_count,
                                                reprint_cmt_count, attitudes_count, mixed_count, pending_approval_count, isLongText, show_mlevel, expire_time,
                                                ad_state, ad_marked, mblogtype, item_category, rid, mlevelSource, cardid, comment_guide_ext, content_auth,
                                                is_show_mixed, safe_tags, comment_manage_info, pic_num, mlevel, pics, bid, user, retweeted_status)
};

struct Card
{
    int card_type;
    std::string profile_type_id;
    std::string itemid;
    std::string scheme;
    Mblog mblog;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Card, card_type, profile_type_id, itemid, scheme, mblog)
};

struct UserMblogData
{
    CardlistInfo cardlistInfo;
    std::list<Card> cards;
    std::string scheme;
    int showAppTips;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UserMblogData, cardlistInfo, cards, scheme, showAppTips)
};

struct UserMblogResponse
{
    int ok;
    std::string msg;
    UserMblogData data;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UserMblogResponse, ok, msg, data)
};

struct ActionLog
{
    int act_type;
    int act_code;
    std::string lcardid;
    std::string fid;
    std::string mid;
    std::string oid;
    std::string uuid;
    std::string source;
    std::string ext;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ActionLog, act_type, act_code, lcardid, fid, mid, oid, uuid, source, ext)
};

struct TopicStruct
{
    std::string title;
    std::string topic_url;
    std::string topic_title;
    ActionLog action_log;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(TopicStruct, title, topic_url, topic_title, action_log)
};

struct TagStruct
{
    std::string tag_name;
    std::string oid;
    int tag_type;
    int tag_hidden;
    std::string tag_scheme;
    std::string url_type_pic;
    ActionLog actionlog;
    std::string bd_object_type;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(TagStruct, tag_name, oid, tag_type, tag_hidden, tag_scheme, url_type_pic, actionlog, bd_object_type)
};

struct ContinueTag
{
    std::string title;
    std::string pic;
    std::string scheme;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ContinueTag, title, pic, scheme)
};

struct PicInfo
{
    std::string url;
    int width;
    int height;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PicInfo, url, width, height)
};

struct PicInfos
{
    PicInfo pic_big;
    PicInfo pic_middle;
    PicInfo pic_small;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PicInfos, pic_big, pic_middle, pic_small)
};

struct Meta
{
    std::string label;
    int quality_index;
    std::string quality_desc;
    std::string quality_label;
    std::string quality_class;
    int type;
    int quality_group;
    bool is_hidden;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Meta, label, quality_index, quality_desc, quality_label, quality_class, type, quality_group, is_hidden)
};

struct PlayInfo
{
    int type;
    std::string mime;
    std::string protocol;
    std::string label;
    std::string url;
    int bitrate;
    std::string prefetch_range;
    std::string video_codecs;
    int fps;
    int width;
    int height;
    int size;
    double duration;
    std::string sar;
    std::string audio_codecs;
    int audio_sample_rate;
    std::string quality_label;
    std::string quality_class;
    std::string quality_desc;
    int audio_channels;
    std::string audio_sample_fmt;
    int audio_bits_per_sample;
    std::string watermark;
    std::string video_decoder;
    bool prefetch_enabled;
    int tcp_receive_buffer;
    bool dolby_atmos;
    int stereo_video;
    int first_pkt_end_pos;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlayInfo, type, mime, protocol, label, url, bitrate, prefetch_range, video_codecs, fps, width, height, size,
                                                duration, sar, audio_codecs, audio_sample_rate, quality_label, quality_class, quality_desc, audio_channels,
                                                audio_sample_fmt, audio_bits_per_sample, watermark, video_decoder, prefetch_enabled, tcp_receive_buffer,
                                                dolby_atmos, stereo_video, first_pkt_end_pos)
};

struct PlayBackInfo
{
    Meta meta;
    PlayInfo play_info;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlayBackInfo, meta, play_info)
};

struct MediaInfo
{
    std::string name;
    std::string stream_url;
    std::string stream_url_hd;
    std::string format;
    std::string h5_url;
    std::string mp4_sd_url;
    std::string mp4_hd_url;
    std::string h265_mp4_hd;
    std::string h265_mp4_ld;
    std::string inch_4_mp4_hd;
    std::string inch_5_mp4_hd;
    std::string inch_5_5_mp4_hd;
    std::string mp4_720p_mp4;
    std::string hevc_mp4_720p;

    int prefetch_type;
    int prefetch_size;
    int act_status;

    std::string protocol;
    std::string media_id;
    int origin_total_bitrate;

    std::string video_orientation;
    int duration;
    int forward_strategy;

    std::string search_scheme;
    int is_short_video;
    int vote_is_show;
    int belong_collection;

    std::string titles_display_time;
    int show_progress_bar;
    bool show_mute_button;

    std::string next_title;
    std::string kol_title;
    long long video_publish_time;

    int play_loop_type;
    std::string author_mid;
    std::string author_name;

    std::string online_users;
    int online_users_number;

    int ttl;
    std::string storage_type;

    PicInfos big_pic_info;
    std::vector<PlayBackInfo> playback_list;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MediaInfo, name, stream_url, stream_url_hd, format, h5_url, mp4_sd_url, mp4_hd_url, h265_mp4_hd, h265_mp4_ld,
                                                inch_4_mp4_hd, inch_5_mp4_hd, inch_5_5_mp4_hd, mp4_720p_mp4, hevc_mp4_720p, prefetch_type, prefetch_size,
                                                act_status, protocol, media_id, origin_total_bitrate, video_orientation, duration, forward_strategy,
                                                search_scheme, is_short_video, vote_is_show, belong_collection, titles_display_time, show_progress_bar,
                                                show_mute_button, next_title, kol_title, video_publish_time, play_loop_type, author_mid, author_name,
                                                online_users, online_users_number, ttl, storage_type, big_pic_info, playback_list)
};

struct MediaInfoItemData
{
    std::string type;
    std::string page_id;
    std::string object_type;
    std::string object_id;
    std::string content1;
    std::string content2;
    int act_status;
    std::string page_pic;
    std::string page_title;
    std::string page_url;
    PicInfos pic_info;

    std::string oid;
    std::string author_id;
    std::string short_url;

    ActionLog action_log;
    MediaInfo media_info;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MediaInfoItemData, type, page_id, object_type, object_id, content1, content2, act_status, page_pic, page_title,
                                                page_url, pic_info, oid, author_id, short_url, action_log, media_info)
};

struct MediaInfoItem
{
    std::string type;
    std::string id;
    MediaInfoItemData data;
    ActionLog actionLog;
    std::string scheme;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MediaInfoItem, type, id, data, actionLog, scheme)
};

struct MixMediaInfo
{
    std::vector<MediaInfoItem> items;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MixMediaInfo, items)
};

struct WeiboAjaxData
{
    std::string created_at;
    int64_t id;
    std::string idstr;
    std::string mid;
    std::string mblogid;
    bool can_edit;
    int edit_count;
    int textLength;
    std::string source;
    bool favorited;
    std::string rid;
    std::string cardid;
    std::vector<std::string> pic_ids;
    int pic_num;
    bool is_paid;
    std::string pic_bg_new;
    int mblog_vip_type;
    NumberDisplayStrategy number_display_strategy;
    int reposts_count;
    int comments_count;
    int attitudes_count;
    int attitudes_status;
    ContinueTag continue_tag;
    bool isLongText;
    int mlevel;
    int content_auth;
    int is_show_bulletin;
    CommentManageInfo comment_manage_info;
    int share_repost_type;
    int mblogtype;
    bool showFeedRepost;
    bool showFeedComment;
    bool pictureViewerSign;
    bool showPictureViewer;
    std::string analysis_extra;
    std::string readtimetype;
    int mixed_count;
    bool is_show_mixed;
    bool isSinglePayAudio;
    std::string text;
    std::string text_raw;
    std::string region_name;
    int ok;
    std::vector<TopicStruct> topic_struct;
    std::vector<TagStruct> tag_struct;
    MixMediaInfo mix_media_info;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WeiboAjaxData, created_at, id, idstr, mid, mblogid, can_edit, edit_count, textLength, source, favorited, rid,
                                                cardid, pic_ids, pic_num, is_paid, pic_bg_new, mblog_vip_type, number_display_strategy, reposts_count,
                                                comments_count, attitudes_count, attitudes_status, continue_tag, isLongText, mlevel, content_auth,
                                                is_show_bulletin, comment_manage_info, share_repost_type, mblogtype, showFeedRepost, showFeedComment,
                                                pictureViewerSign, showPictureViewer, analysis_extra, readtimetype, mixed_count, is_show_mixed,
                                                isSinglePayAudio, text, text_raw, region_name, ok, topic_struct, tag_struct, mix_media_info)
};

struct Scrubber
{
    int width;
    int height;
    int col;
    int row;
    int interval;
    std::vector<std::string> urls;

    friend void to_json(nlohmann::json& nlohmann_json_j, const Scrubber& nlohmann_json_t)
    {
        nlohmann_json_j["width"] = nlohmann_json_t.width;
        nlohmann_json_j["height"] = nlohmann_json_t.height;
        nlohmann_json_j["col"] = nlohmann_json_t.col;
        nlohmann_json_j["row"] = nlohmann_json_t.row;
        nlohmann_json_j["interval"] = nlohmann_json_t.interval;
        nlohmann_json_j["urls"] = nlohmann_json_t.urls;
    }

    friend void from_json(const nlohmann::json& nlohmann_json_j, Scrubber& nlohmann_json_t)
    {
        if (!nlohmann_json_j.is_object())
        {
            return;
        }

        const Scrubber nlohmann_json_default_obj{};
        nlohmann_json_t.width = nlohmann_json_j.value("width", nlohmann_json_default_obj.width);
        nlohmann_json_t.height = nlohmann_json_j.value("height", nlohmann_json_default_obj.height);
        nlohmann_json_t.col = nlohmann_json_j.value("col", nlohmann_json_default_obj.col);
        nlohmann_json_t.row = nlohmann_json_j.value("row", nlohmann_json_default_obj.row);
        nlohmann_json_t.interval = nlohmann_json_j.value("interval", nlohmann_json_default_obj.interval);
        nlohmann_json_t.urls = nlohmann_json_j.value("urls", nlohmann_json_default_obj.urls);
    }
};

struct Reward
{
    int version_state;
    int state;
    int welfare;
    std::string desc;
    std::string reward_button_scheme;
    std::string reward_params;
    std::string mid;
    User user;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Reward, version_state, state, welfare, desc, reward_button_scheme, reward_params, mid, user)
};

struct WeiboUrls
{
    std::map<std::string, std::string> urls;

    friend void to_json(nlohmann::json& nlohmann_json_j, const WeiboUrls& nlohmann_json_t)
    {
        nlohmann_json_j = nlohmann_json_t.urls;
    }
    friend void from_json(const nlohmann::json& nlohmann_json_j, WeiboUrls& nlohmann_json_t)
    {
        try
        {
            if (nlohmann_json_j.is_object())
            {
                nlohmann_json_t.urls.clear();
                for (const auto& [key, value] : nlohmann_json_j.items())
                {
                    nlohmann_json_t.urls[key] = value.get<std::string>();
                }
            }
            else if (nlohmann_json_j.is_array())
            {
                nlohmann_json_t.urls.clear();
                for (const auto& item : nlohmann_json_j)
                {
                    if (!item.is_object())
                    {
                        continue;
                    }
                    for (auto it = item.begin(); it != item.end(); ++it)
                    {
                        nlohmann_json_t.urls[it.key()] = it.value().get<std::string>();
                    }
                }
            }
            else
            {
                return;
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error parsing WeiboUrls: " << e.what() << std::endl;
        }
    }
};

struct ComponentPlayPlayinfo
{
    int64_t mid;
    std::string id;
    std::string oid;
    int64_t media_id;

    User user;

    bool is_follow;
    bool attitude;

    std::string date;
    int64_t real_date;
    std::string idstr;
    std::string author;
    int64_t author_id;
    std::string nickname;

    bool verified;
    int verified_type;
    int verified_type_ext;
    std::string verified_reason;

    std::string avatar;
    std::string followers_count;

    std::string reposts_count;
    std::string comments_count;
    int attitudes_count;

    std::string title;
    WeiboUrls urls;

    std::string cover_image;
    std::string duration;
    int duration_time;

    int play_start;
    int play_start_time;

    std::string uuid;
    std::string text;
    std::string url_short;

    int is_show_bulletin;

    CommentManageInfo comment_manage_info;

    std::string video_orientation;

    int is_contribution;
    bool live;
    int cooperate_video;
    int is_pay_audio;
    bool is_paid;

    std::string stream_url;
    std::string object_type;
    std::string play_count;
    std::string ip_info_str;
    std::string attitude_dynamic_adid;
    int user_video_count;
    bool allow_comment;

    Scrubber scrubber;

    Reward reward;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ComponentPlayPlayinfo, mid, id, oid, media_id, user, is_follow, attitude, date, real_date, idstr, author,
                                                author_id, nickname, verified, verified_type, verified_type_ext, verified_reason, avatar, followers_count,
                                                reposts_count, comments_count, attitudes_count, title, urls, cover_image, duration, duration_time, play_start,
                                                play_start_time, uuid, text, url_short, is_show_bulletin, comment_manage_info, video_orientation,
                                                is_contribution, live, cooperate_video, is_pay_audio, is_paid, stream_url, object_type, play_count, ip_info_str,
                                                attitude_dynamic_adid, user_video_count, allow_comment, scrubber, reward)
};

struct ComponentPlayPlayinfoResponse
{
    std::string code;
    std::string msg;
    struct ComponentPlayPlayinfoItem
    {
        ComponentPlayPlayinfo Component_Play_Playinfo;
        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ComponentPlayPlayinfoItem, Component_Play_Playinfo)
    } data;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ComponentPlayPlayinfoResponse, code, msg, data)
};

struct TabInfo
{
    std::string name;
    std::string tabName;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(TabInfo, name, tabName)
};

struct UserInfo
{
    User user;
    std::vector<TabInfo> tabList;
    std::string blockText;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UserInfo, user, tabList, blockText)
};

struct UserInfoResponse
{
    int ok;
    UserInfo data;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UserInfoResponse, ok, data)
};

}  // namespace weiboapi
