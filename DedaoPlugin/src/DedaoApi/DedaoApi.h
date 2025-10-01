#pragma once
#include <nlohmann/json.hpp>
#include <list>
#include <iostream>

namespace dedaoapi
{

template <typename T>
struct LoginResponse
{
    int errCode;
    std::string errMsg;
    T data;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LoginResponse<T>, errCode, errMsg, data)
};

struct QRData
{
    std::string qrCode;
    std::string qrCodeString;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(QRData, qrCode, qrCodeString)
};

using LoginQrcode = LoginResponse<QRData>;

struct LoginStatusCheck
{
    int status{};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LoginStatusCheck, status)
};

using LoginCheck = LoginResponse<LoginStatusCheck>;

struct RequestInfo
{
    int c{};
    std::string e;
    int s{};
    int t{};
    std::string apm;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(RequestInfo, c, e, s, t, apm)
};

template <typename T>
struct DedaoResponse
{
    RequestInfo h;
    T c;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(DedaoResponse<T>, h, c)
};

struct VipUser
{
    std::string info;
    int stat{};
    uint64_t user_id;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(VipUser, info, stat, user_id)
};

struct UserInfo
{
    std::string nickname;
    std::string avatar;
    int today_study_time{};
    int study_serial_days{};
    int is_v{};
    VipUser vip_user;
    int is_teacher{};
    std::string uid_hazy;
    std::string watermark;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UserInfo, nickname, avatar, today_study_time, study_serial_days, is_v, vip_user, is_teacher, uid_hazy,
                                                watermark)
};

using UserInfoResponse = DedaoResponse<UserInfo>;

struct Teacher
{
    long long teacher_uid{};
    std::string teacher_name;
    std::string teacher_intro;
    std::string avatar;
    std::string profile_url;
    int is_bigv{};
    int fllow_status{};
    int follow_count{};
    int fans_count{};
    std::string teacher_uid_alias;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Teacher, teacher_uid, teacher_name, teacher_intro, avatar, profile_url, is_bigv, fllow_status, follow_count,
                                                fans_count, teacher_uid_alias)
};

struct LiveOutline
{
    std::string title;
    std::string content;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LiveOutline, title, content)
};

struct Booking
{
    int bookStatus{};
    int totalNum{};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Booking, bookStatus, totalNum)
};

struct LiveSeriesInfo
{
    int id{};
    std::string title;
    std::string info;
    std::string image;
    int status{};
    std::string ddurl;
    int playback_count{};
    int live_count{};
    std::string create_time;
    std::string update_time;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LiveSeriesInfo, id, title, info, image, status, ddurl, playback_count, live_count, create_time, update_time)
};

struct LiveArticleInfo
{
    int article_id{};
    std::string article_detail_ddurl;
    std::string article_alias_id;
    std::string article_ptype_alias_id;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LiveArticleInfo, article_id, article_detail_ddurl, article_alias_id, article_ptype_alias_id)
};

struct LiveActivityInfo
{
    std::string live_activity_ddurl;
    int live_activity_type{};
    int live_playback_disabled{};
    std::string live_activity_id;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LiveActivityInfo, live_activity_ddurl, live_activity_type, live_playback_disabled, live_activity_id)
};

struct LiveDetail
{
    std::vector<Teacher> live_lecture_list;
    LiveOutline live_outline;
    std::string room_detail_ddurl;
    int status{};
    std::string share_image_square;
    std::string logo;
    std::string poster_image;
    std::string intro;
    std::string title;
    long long starttimestamp{};
    std::string share_title;
    std::string share_summary;
    std::string summary;
    int id{};
    std::string share_url;
    std::string share_wechat_type;
    std::string share_wechat_channel;
    std::string share_wechat_channel_start_time;
    Booking booking;
    std::string live_series_ids;
    int playback_status{};
    int playback_duration{};
    std::string playback_duration_text;
    int publish_status{};
    LiveSeriesInfo live_serires_info;
    std::string live_type;
    int ptype{};
    int has_quiz_activity{};
    std::string quiz_activity_id;
    int next_live_id{};
    std::string last_start_time;
    std::string last_end_time;
    int max_num{};
    int live_pv{};
    int live_viewers{};
    bool is_privilege_live{};
    std::string author_text;
    LiveArticleInfo live_article_info;
    LiveActivityInfo live_activity_info;
    bool is_activity_live{};
    std::string youzan_vip_product_url;
    int live_uv{};
    int live_privilege_type{};
    std::string live_record_num;
    int theme_type{};
    std::string live_dp_code;
    bool has_outliner{};
    std::string live_dp_name;
    int online_number{};
    std::string video_icon;
    int share_count{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LiveDetail, live_lecture_list, live_outline, room_detail_ddurl, status, share_image_square, logo, poster_image,
                                                intro, title, starttimestamp, share_title, share_summary, summary, id, share_url, share_wechat_type,
                                                share_wechat_channel, share_wechat_channel_start_time, booking, live_series_ids, playback_status,
                                                playback_duration, playback_duration_text, publish_status, live_serires_info, live_type, ptype,
                                                has_quiz_activity, quiz_activity_id, next_live_id, last_start_time, last_end_time, max_num, live_pv,
                                                live_viewers, is_privilege_live, author_text, live_article_info, live_activity_info, is_activity_live,
                                                youzan_vip_product_url, live_uv, live_privilege_type, live_record_num, theme_type, live_dp_code, has_outliner,
                                                live_dp_name, online_number, video_icon, share_count)
};

using LiveInfo = DedaoResponse<LiveDetail>;

struct LiveToken
{
    int status{};
    std::string token;
    std::string live_privilege_tips;
    std::string live_type;
    std::string logo;
    int expired_seconds{};
    int product_type{};
    int product_id{};
    std::string ddurl;
    bool need_buy{};
    std::string button_text;
    int product_subid{};
    std::string live_product_buy_tips;
    int invite_count{};
    bool is_privilege_live{};
    std::string error_msg;
    bool has_buy{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LiveToken, status, token, live_privilege_tips, live_type, logo, expired_seconds, product_type, product_id,
                                                ddurl, need_buy, button_text, product_subid, live_product_buy_tips, invite_count, is_privilege_live, error_msg,
                                                has_buy)
};

using LiveTokenInfo = DedaoResponse<LiveToken>;

struct PlaybackInfo
{
    std::vector<std::string> ld;
    std::vector<std::string> hd;
    std::vector<std::string> ud;
    std::vector<std::string> sd;
    std::vector<std::string> audio;
    std::string token;
    std::string token_version;
    std::vector<std::string> screen_projection_url;
    int duration{};
    std::string duration_text;
    int playback_status{};
    long long ddmedia_id{};
    std::string playback_video_transcoding_text;
    std::string web_pc_media_token;
    std::string ld_m3u8;
    std::string hd_m3u8;
    std::string audio_m3u8;
    std::string caption;
    std::string caption_vtt;
    int caption_position{};
    std::string vtt_url;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlaybackInfo, ld, hd, ud, sd, audio, token, token_version, screen_projection_url, duration, duration_text,
                                                playback_status, ddmedia_id, playback_video_transcoding_text, web_pc_media_token, ld_m3u8, hd_m3u8, audio_m3u8,
                                                caption, caption_vtt, caption_position, vtt_url)
};

struct ReportData
{
    std::string base_item_id;
    std::string base_item_type;
    std::string main_item_id;
    std::string main_item_type;
    int product_id{};
    int product_type{};
    std::string product_id_str;
    std::string product_type_str;
    std::string base_item_id_str;
    std::string main_item_id_str;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ReportData, base_item_id, base_item_type, main_item_id, main_item_type, product_id, product_type,
                                                product_id_str, product_type_str, base_item_id_str, main_item_id_str)
};

struct PrivilegeProducts
{
    int id{};
    int product_id{};
    int product_sub_id{};
    int product_type{};
    std::string title;
    std::string ddurl;
    int live_start_time{};
    int live_id{};
    bool has_privilege{};
    std::string alias_name;
    std::string alias_id;
    bool is_privilege_live{};
    int product_group_id{};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PrivilegeProducts, id, product_id, product_sub_id, product_type, title, ddurl, live_start_time, live_id,
                                                has_privilege, alias_name, alias_id, is_privilege_live, product_group_id)
};

struct LiveTab
{
    std::string title;
    std::string tab_type;
    std::string ddurl;
    int sort{};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LiveTab, title, tab_type, ddurl, sort)
};

struct LiveRoomInfo
{
    int id{};
    std::string title;
    std::string author;
    std::string intro;
    std::string starttime;
    std::string push_time;
    std::string share_title;
    std::string share_summary;
    std::string logo;
    std::string share_image;
    std::string share_image_square;
    int status{};
    std::string createtime;
    std::string udt;
    std::string type;
    std::string last_start_time;
    std::string last_end_time;
    std::string amount;
    long long starttimestamp{};
    long long timestamp{};
    Booking booking;
    int log_id{};
    std::string log_type;
    std::vector<std::string> ld;
    std::vector<std::string> sd;
    std::vector<std::string> hd;
    std::vector<std::string> ud;
    std::vector<std::string> only_audio;
    std::string log_interface;
    std::string alias_id;
    std::string ld_m3u8;
    std::string home_img;
    std::string hd_m3u8;
    std::string summary;
    int live_duration{};
    std::string live_duration_text;
    PlaybackInfo playback_info;
    std::string minibar_stream_url;
    ReportData report_data_live;
    ReportData report_data_video;
    PrivilegeProducts PrivilegeProduct;
    std::string dd_url;
    LiveActivityInfo live_activity_info;
    std::string rtmp_ld;
    std::string rtmp_hd;
    std::string sd_m3u8;
    std::string ud_m3u8;
    std::string live_md;
    std::string live_ud;
    std::string live_hd;
    std::string live_sd;
    std::vector<LiveTab> live_tab_list;
    int location_msg_status{};
    std::string home_img_v2;
    int use_subtitles{};
    std::string activity_encrypt_id;
    int viewers_count{};
    std::string ai_watermark_icon;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LiveRoomInfo, id, title, author, intro, starttime, push_time, share_title, share_summary, logo, share_image,
                                                share_image_square, status, createtime, udt, type, last_start_time, last_end_time, amount, starttimestamp,
                                                timestamp, booking, log_id, log_type, ld, sd, hd, ud, only_audio, log_interface, alias_id, ld_m3u8, home_img,
                                                hd_m3u8, summary, live_duration, live_duration_text, playback_info, minibar_stream_url, report_data_live,
                                                report_data_video, PrivilegeProduct, dd_url, live_activity_info, rtmp_ld, rtmp_hd, sd_m3u8, ud_m3u8, live_md,
                                                live_ud, live_hd, live_sd, live_tab_list, location_msg_status, home_img_v2, use_subtitles, activity_encrypt_id,
                                                viewers_count, ai_watermark_icon)
};

using RoomDetail = DedaoResponse<LiveRoomInfo>;

struct Format
{
    std::string type;
    std::string format;
    int resolution{};
    std::string tag;
    std::string url;
    int size{};
    std::string etag;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Format, type, format, resolution, tag, url, size, etag)

    auto operator<=>(const Format& other) const
    {
        return resolution <=> other.resolution;
    }
};

struct Track
{
    int track_id{};
    int track_type{};
    std::string track_type_tag;
    int duration{};
    std::vector<Format> formats;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Track, track_id, track_type, track_type_tag, duration, formats)
};

struct MediaInfo
{
    std::string media_alias_id;
    std::string last_modify;
    int version_id{};
    std::vector<Track> tracks;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MediaInfo, media_alias_id, last_modify, version_id, tracks)
};

using MediaStreamInfo = DedaoResponse<MediaInfo>;

struct ClassInfo
{
    std::string log_id;
    std::string log_type;
    int id;
    std::string id_str;
    std::string enid;
    int product_id;
    int product_type;
    int has_chapter;
    std::string name;
    std::string intro;
    int phase_num;
    int learn_user_count;
    int current_article_count;
    std::string highlight;
    std::string index_img;
    std::string index_img_applet;
    std::string logo;
    std::string logo_iphonex;
    std::string square_img;
    std::string outline_img;
    std::string player_img;
    std::string share_title;
    std::string share_summary;
    int status;
    int order_num;
    std::string shzf_url;
    std::string shzf_url_qr;
    std::string price_desc;
    int64_t article_time;
    std::string article_title;
    int is_subscribe;
    int lecturer_uid;
    std::string lecturer_name;
    std::string lecturer_title;
    std::string lecturer_intro;
    std::string lecturer_name_and_title;
    std::string lecturer_avatar;
    int is_finished;
    int64_t update_time;
    std::string share_url;
    std::string presale_url;
    bool without_audio;
    int view_type;
    std::string h5_url_name;
    bool package_manager_switch;
    bool package_manager;
    int64_t estimated_shelf_time;
    int64_t estimated_down_time;
    std::string corner_img;
    std::string corner_img_vertical;
    bool without_giving;
    std::string dd_url;
    std::string dd_media_id;
    std::string video_cover;
    int formal_article_count;
    int video_class;
    std::string video_class_intro;
    std::string activity_icon;
    int reality_extra_count;
    int reality_formal_count;
    std::vector<int> intro_article_ids;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ClassInfo, log_id, log_type, id, id_str, enid, product_id, product_type, has_chapter, name, intro, phase_num,
                                                learn_user_count, current_article_count, highlight, index_img, index_img_applet, logo, logo_iphonex, square_img,
                                                outline_img, player_img, share_title, share_summary, status, order_num, shzf_url, shzf_url_qr, price_desc,
                                                article_time, article_title, is_subscribe, lecturer_uid, lecturer_name, lecturer_title, lecturer_intro,
                                                lecturer_name_and_title, lecturer_avatar, is_finished, update_time, share_url, presale_url, without_audio,
                                                view_type, h5_url_name, package_manager_switch, package_manager, estimated_shelf_time, estimated_down_time,
                                                corner_img, corner_img_vertical, without_giving, dd_url, dd_media_id, video_cover, formal_article_count,
                                                video_class, video_class_intro, activity_icon, reality_extra_count, reality_formal_count, intro_article_ids)
};

struct ClassVideo
{
    std::string token;
    int token_version;
    std::string cover_img;
    uint64_t dd_media_id;
    std::string dd_media_id_str;
    std::string m3u8_token;
    int duration;
    std::string bitrate_480;
    int bitrate_480_size;
    std::string bitrate_480_audio;
    std::string bitrate_720;
    int bitrate_720_size;
    std::string bitrate_720_audio;
    std::string bitrate_1080;
    int bitrate_1080_size;
    std::string bitrate_1080_audio;
    bool is_drm;
    int listen_progress;
    bool listen_finished;
    std::string log_id;
    std::string log_type;
    std::string caption;
    std::string vtt_caption;
    std::string video_audio;
    std::string source_name;
    int source_type;
    int source_id;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ClassVideo, token, token_version, cover_img, dd_media_id, dd_media_id_str, m3u8_token, duration, bitrate_480,
                                                bitrate_480_size, bitrate_480_audio, bitrate_720, bitrate_720_size, bitrate_720_audio, bitrate_1080,
                                                bitrate_1080_size, bitrate_1080_audio, is_drm, listen_progress, listen_finished, log_id, log_type, caption,
                                                vtt_caption, video_audio, source_name, source_type, source_id)
};

struct VStat
{
    int stat;
    std::string info;
    std::string cancel_info;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(VStat, stat, info, cancel_info)
};

struct Comment
{
    uint64_t id;
    uint64_t user_id;
    std::string title;
    int score;
    std::string content;
    std::string no_style_content;
    std::string note_id;
    std::string nickname;
    std::string avatar;
    std::string avatar_s;
    VStat vstat;
    std::string track_info;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Comment, id, user_id, title, score, content, no_style_content, note_id, nickname, avatar, avatar_s, vstat,
                                                track_info)
};

struct CommentInfo
{
    std::vector<Comment> comment_list;
    int count{};
    std::string average_score;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CommentInfo, comment_list, average_score)
};

struct MediaBaseInfo
{
    int media_type{};
    std::string source_id;
    std::string security_token;
    std::string source_url;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MediaBaseInfo, media_type, source_id, security_token, source_url)
};

struct AudioInfo
{
    std::string alias_id;
    std::string m3u8_token;
    std::string extrainfo;
    int class_id{};
    std::string title;
    std::string share_title;
    std::string mp3_play_url;
    int duration{};
    int schedule{};
    int topic_id{};
    std::string summary;
    int price{};
    std::string icon;
    int size{};
    std::string etag;
    std::string token;
    std::string share_summary;
    int collection{};
    int count{};
    int bored_count{};
    int audio_type{};
    int drm_version{};
    int source_id{};
    int source_type{};
    std::string source_icon;
    std::string source_name;
    int listen_progress{};
    bool listen_finished{};
    std::string dd_article_id;
    std::string audio_list_icon;
    std::string class_course_id;
    std::string class_article_id;
    std::string log_id;
    std::string log_type;
    std::string log_interface;
    std::string trackinfo;
    int used_drm{};
    std::string index_img;
    std::string reader;
    std::string reader_name;
    std::string player_img;
    bool has_play_auth{};
    bool is_vip{};
    int category{};
    int is_free{};
    int trial_duration{};
    std::string update_tips;
    std::string trial_listen_tips;
    std::string trial_listen_end_tips;
    std::string trial_listen_end_url;
    int play_count{};
    std::string play_count_tips;
    int book_shelf_status{};
    std::string share_url;
    std::string play_dd_url;
    bool is_subscribed{};
    int package_pid{};
    int package_ptype{};
    std::string package_title;
    std::string superscript;
    std::string podcast_icon;
    int is_newest{};
    int is_latest_learning{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AudioInfo, alias_id, m3u8_token, extrainfo, class_id, title, share_title, mp3_play_url, duration, schedule,
                                                topic_id, summary, price, icon, size, etag, token, share_summary, collection, count, bored_count, audio_type,
                                                drm_version, source_id, source_type, source_icon, source_name, listen_progress, listen_finished, dd_article_id,
                                                audio_list_icon, class_course_id, class_article_id, log_id, log_type, log_interface, trackinfo, used_drm,
                                                index_img, reader, reader_name, player_img, has_play_auth, is_vip, category, is_free, trial_duration,
                                                update_tips, trial_listen_tips, trial_listen_end_tips, trial_listen_end_url, play_count, play_count_tips,
                                                book_shelf_status, share_url, play_dd_url, is_subscribed, package_pid, package_ptype, package_title,
                                                superscript, podcast_icon, is_newest, is_latest_learning)
};

struct Article
{
    uint64_t id;
    std::string id_str;
    std::string enid;
    std::string class_enid;
    uint64_t origin_id;
    std::string origin_id_str;
    int product_type;
    int product_id;
    std::string product_id_str;
    int class_id;
    std::string class_id_str;
    int chapter_id;
    std::string chapter_id_str;
    std::string title;
    std::string logo;
    std::string url;
    std::string summary;
    int mold;
    std::string push_content;
    int64_t publish_time;
    int64_t push_time;
    int push_status;
    std::string share_title;
    std::string share_content;
    int share_switch;
    uint64_t dd_article_id;
    std::string dd_article_id_str;
    std::string dd_article_token;
    int status;
    int64_t create_time;
    int64_t update_time;
    int cur_learn_count;
    bool is_free_try;
    bool is_user_free_try;
    int order_num;
    bool is_like;
    std::string share_url;
    std::string trial_share_url;
    bool is_read;
    std::string log_id;
    std::string log_type;
    std::string recommend_title;
    AudioInfo audio;
    bool is_buy;
    uint64_t dd_media_id;
    std::string dd_media_id_str;
    int video_status;
    std::vector<MediaBaseInfo> media_base_info;
    uint64_t dd_live_id;
    int not_join_plan;
    bool is_red_packet_try;
    std::string red_packet_expire_time;
    bool is_show_play_later_button;
    bool is_add_play_later;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Article, id, id_str, enid, class_enid, origin_id, origin_id_str, product_type, product_id, product_id_str,
                                                class_id, class_id_str, chapter_id, chapter_id_str, title, logo, url, summary, mold, push_content, publish_time,
                                                push_time, push_status, share_title, share_content, share_switch, dd_article_id, dd_article_id_str,
                                                dd_article_token, status, create_time, update_time, cur_learn_count, is_free_try, is_user_free_try, order_num,
                                                is_like, share_url, trial_share_url, is_read, log_id, log_type, recommend_title, audio, is_buy, dd_media_id,
                                                dd_media_id_str, video_status, media_base_info, dd_live_id, not_join_plan, is_red_packet_try,
                                                red_packet_expire_time, is_show_play_later_button, is_add_play_later)
};

struct Chapter
{
    uint64_t id;
    std::string id_str;
    int class_id;
    std::string class_id_str;
    std::string order_name;
    std::string name;
    std::string intro;
    int phase_num;
    int status;
    int order_num;
    int is_finished;
    int64_t update_time;
    std::string log_id;
    std::string log_type;
    std::vector<Article> article_list;
    int appendage_count;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Chapter, id, id_str, class_id, class_id_str, order_name, name, intro, phase_num, status, order_num, is_finished,
                                                update_time, log_id, log_type, article_list, appendage_count)
};

struct LectureUserInfo
{
    uint64_t userId;
    std::string name;
    std::string avatar;
    std::string slogan;
    int vStat;
    std::string vStatInto;
    int follow;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LectureUserInfo, userId, name, avatar, slogan, vStat, vStatInto, follow)
};

struct LecturerDedaoShare
{
    std::string share_id;
    VipUser vip_user;
    LectureUserInfo lecture_user_info;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LecturerDedaoShare, share_id, vip_user, lecture_user_info)
};

struct Course
{
    ClassInfo class_info;
    ClassVideo class_video;
    CommentInfo class_comment_info;
    std::vector<Chapter> chapter_list;
    LecturerDedaoShare lecturer_dedao_share;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Course, class_info, class_video, class_comment_info, chapter_list, lecturer_dedao_share)
};

using CourseInfo = DedaoResponse<Course>;

struct ArticleList
{
    std::vector<Article> article_list;
    int class_id{};
    int pid{};
    int ptype{};
    bool reverse{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ArticleList, article_list, class_id, pid, ptype, reverse)
};

using ArticleInfo = DedaoResponse<ArticleList>;

struct V2Token
{
    std::string token;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(V2Token, token)
};

using V2TokenInfo = DedaoResponse<V2Token>;

}  // namespace dedaoapi
