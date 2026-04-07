#pragma once
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace douyinapi
{

template <typename Result>
struct DouYinBasicResponse
{
    int code{};
    bool success{};
    std::string message;
    Result data;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(DouYinBasicResponse<Result>, code, success, message, data)
};

struct FrontierParams
{
    std::string access_key;
    std::string frontier_device;
    int method{};
    int product_id{};
    int service_id{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FrontierParams, access_key, frontier_device, method, product_id, service_id)
};

struct QRCode
{
    std::string app_name;
    std::string captcha;
    std::string copywriting;
    std::string desc_url;
    std::string description;
    int error_code{};
    long long expire_time{};
    FrontierParams frontier_params;
    bool is_frontier{};
    std::string qrcode;
    std::string qrcode_index_url;
    std::string token;
    std::string web_name;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(QRCode, app_name, captcha, copywriting, desc_url, description, error_code, expire_time, frontier_params,
                                                is_frontier, qrcode, qrcode_index_url, token, web_name)
};

using QRCodeResponse = DouYinBasicResponse<QRCode>;

struct ScanDeviceInfo
{
    int device_id{};
    int app_id{};
    std::string ip;
    int verify_time{};
    std::string verify_way;
    std::string scan_device_display_name;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ScanDeviceInfo, device_id, app_id, ip, verify_time, verify_way, scan_device_display_name)
};

struct ScanUserInfo
{
    int user_id{};
    std::string screen_name;
    std::string avatar_url;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ScanUserInfo, user_id, screen_name, avatar_url)
};

struct QRCodeStatusData
{
    std::string captcha;
    std::string desc_url;
    std::string description;
    int error_code{};
    std::string extra;
    int scan_app_id{};
    ScanDeviceInfo scan_device_info;
    ScanUserInfo scan_user_info;
    std::string status;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(QRCodeStatusData, captcha, desc_url, description, error_code, extra, scan_app_id, scan_device_info,
                                                scan_user_info, status)
};

using QRCodeStatusResponse = DouYinBasicResponse<QRCodeStatusData>;

struct UrlModel
{
    int height{};
    int width{};
    std::string uri;
    std::string url_key;
    std::vector<std::string> url_list;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UrlModel, height, width, uri, url_key, url_list)
};

struct ShareInfo
{
    std::string share_desc;
    std::string share_desc_info;
    UrlModel share_qrcode_url;
    std::string share_title;
    std::string share_title_myself;
    std::string share_title_other;
    std::string share_link_desc;
    std::string share_url;
    std::string share_weibo_desc;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ShareInfo, share_desc, share_desc_info, share_qrcode_url, share_title, share_title_myself, share_title_other,
                                                share_url, share_weibo_desc)
};

struct Author
{
    std::string account_cert_info;
    UrlModel avatar_thumb;

    std::string awemehts_greet_info;
    int close_friend_type{};
    int contacts_status{};

    std::vector<UrlModel> cover_url;

    int create_time{};
    std::string custom_verify;

    std::string enterprise_verify_reason;
    int favoriting_count{};
    int follow_status{};
    int follower_count{};

    int follower_status{};
    int following_count{};

    bool is_ad_fake{};
    bool is_blocked_v2{};
    bool is_blocking_v2{};
    int is_cf{};
    int live_high_value{};
    int mate_add_permission{};
    int max_follower_count{};

    std::string nickname;
    bool prevent_download{};
    std::string risk_notice_text;

    std::string sec_uid;
    int secret{};

    ShareInfo share_info;

    std::string short_id;
    std::string signature;

    int special_follow_status{};

    int status{};
    int story25_comment{};
    int story_interactive{};
    int story_ttl{};

    long long total_favorited{};
    std::string uid;
    std::string unique_id;

    int user_age{};
    bool user_canceled{};
    int verification_type{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Author, account_cert_info, avatar_thumb, awemehts_greet_info, close_friend_type, contacts_status, cover_url,
                                                create_time, custom_verify, enterprise_verify_reason, favoriting_count, follow_status, follower_count,
                                                follower_status, following_count, is_ad_fake, is_blocked_v2, is_blocking_v2, is_cf, live_high_value,
                                                mate_add_permission, max_follower_count, nickname, prevent_download, risk_notice_text, sec_uid, secret,
                                                share_info, short_id, signature, special_follow_status, status, story25_comment, story_interactive, story_ttl,
                                                total_favorited, uid, unique_id, user_age, user_canceled, verification_type)
};

using AccountInfoResponse = DouYinBasicResponse<Author>;

struct AwemeControl
{
    bool can_comment{};
    bool can_forward{};
    bool can_share{};
    bool can_show_comment{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AwemeControl, can_comment, can_forward, can_share, can_show_comment)
};

struct SearchImpr
{
    std::string entity_id;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SearchImpr, entity_id)
};

struct Music
{
    std::string album;
    int audition_duration{};
    std::string author;
    bool author_deleted{};
    int author_status{};

    UrlModel avatar_large;
    UrlModel avatar_medium;
    UrlModel avatar_thumb;

    int binded_challenge_id{};
    bool can_background_play{};
    int collect_stat{};

    UrlModel cover_hd;
    UrlModel cover_large;
    UrlModel cover_medium;
    UrlModel cover_thumb;

    bool dmv_auto_show{};
    int dsp_status{};
    int duration{};
    int end_time{};

    std::string extra;

    long long id{};
    std::string id_str;

    bool is_audio_url_with_cookie{};
    bool is_commerce_music{};
    bool is_del_video{};
    bool is_matched_metadata{};
    bool is_original{};
    bool is_original_sound{};
    bool is_pgc{};
    bool is_restricted{};
    bool is_video_self_see{};

    std::string mid;

    int music_collect_count{};
    std::string music_cover_atmosphere_color_value;
    int music_status{};

    bool mute_share{};
    std::string offline_desc;

    std::string owner_handle;
    std::string owner_id;
    std::string owner_nickname;

    int pgc_music_type{};

    UrlModel play_url;

    bool prevent_download{};
    int prevent_item_download_status{};

    int preview_end_time{};
    int preview_start_time{};
    int reason_type{};

    bool redirect{};
    std::string schema_url;

    SearchImpr search_impr;

    std::string sec_uid;

    int shoot_duration{};
    bool show_origin_clip{};
    int source_platform{};

    int start_time{};
    int status{};

    UrlModel strong_beat_url;

    std::string title;
    int user_count{};
    int video_duration{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Music, album, audition_duration, author, author_deleted, author_status, avatar_large, avatar_medium,
                                                avatar_thumb, binded_challenge_id, can_background_play, collect_stat, cover_hd, cover_large, cover_medium,
                                                cover_thumb, dmv_auto_show, dsp_status, duration, end_time, extra, id, id_str, is_audio_url_with_cookie,
                                                is_commerce_music, is_del_video, is_matched_metadata, is_original, is_original_sound, is_pgc, is_restricted,
                                                is_video_self_see, mid, music_collect_count, music_cover_atmosphere_color_value, music_status, mute_share,
                                                offline_desc, owner_handle, owner_id, owner_nickname, pgc_music_type, play_url, prevent_download,
                                                prevent_item_download_status, preview_end_time, preview_start_time, reason_type, redirect, schema_url,
                                                search_impr, sec_uid, shoot_duration, show_origin_clip, source_platform, start_time, status, strong_beat_url,
                                                title, user_count, video_duration)
};

struct BigThumb
{
    double duration{};
    std::string fext;
    int img_num{};
    std::string img_url;
    std::vector<std::string> img_urls;

    int img_x_len{};
    int img_x_size{};
    int img_y_len{};
    int img_y_size{};

    double interval{};
    std::string uri;
    std::vector<std::string> uris;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(BigThumb, duration, fext, img_num, img_url, img_urls, img_x_len, img_x_size, img_y_len, img_y_size, interval,
                                                uri, uris)
};

struct PlayAddr
{
    long long data_size{};
    std::string file_cs;
    std::string file_hash;

    int height{};
    int width{};

    std::string uri;
    std::string url_key;
    std::vector<std::string> url_list;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlayAddr, data_size, file_cs, file_hash, height, width, uri, url_key, url_list)
};

struct BitRate
{
    int FPS{};
    std::string HDR_bit;
    std::string HDR_type;

    int bit_rate{};
    std::string format;
    std::string gear_name;

    int is_bytevc1{};
    int is_h265{};

    PlayAddr play_addr;

    int quality_type{};

    std::string video_extra;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(BitRate, FPS, HDR_bit, HDR_type, bit_rate, format, gear_name, is_bytevc1, is_h265, play_addr, quality_type,
                                                video_extra)
};

struct AudioUrlList
{
    std::string backup_url;
    std::string fallback_url;
    std::string main_url;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AudioUrlList, backup_url, fallback_url, main_url)
};

struct AudioMeta
{
    int bitrate{};
    std::string codec_type;
    std::string encoded_type;
    std::string file_hash;
    std::string file_id;

    std::string format;
    int fps{};
    std::string logo_type;
    std::string media_type;

    std::string quality;
    std::string quality_desc;

    long long size{};

    std::string sub_info;

    AudioUrlList url_list;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AudioMeta, bitrate, codec_type, encoded_type, file_hash, file_id, format, fps, logo_type, media_type, quality,
                                                quality_desc, size, sub_info, url_list)
};

struct BitRateAudio
{
    std::string audio_extra;
    AudioMeta audio_meta;
    int audio_quality{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(BitRateAudio, audio_extra, audio_meta, audio_quality)
};

struct Video
{
    std::vector<BigThumb> big_thumbs;
    std::vector<BitRate> bit_rate;
    std::vector<BitRateAudio> bit_rate_audio;

    long long cdn_url_expired{};

    UrlModel cover;
    UrlModel cover_original_scale;

    PlayAddr download_addr;
    PlayAddr download_suffix_logo_addr;

    int duration{};

    UrlModel dynamic_cover;

    std::string format;

    UrlModel gaussian_cover;

    bool has_download_suffix_logo_addr{};
    bool has_watermark{};

    int height{};
    int width{};

    int horizontal_type{};
    int is_h265{};
    int is_long_video{};
    int is_source_HDR{};

    std::string meta;

    std::string misc_download_addrs;

    UrlModel origin_cover;

    PlayAddr play_addr;
    PlayAddr play_addr_265;
    PlayAddr play_addr_h264;

    std::string ratio;
    std::string video_model;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Video, big_thumbs, bit_rate, bit_rate_audio, cdn_url_expired, cover, cover_original_scale, download_addr,
                                                download_suffix_logo_addr, duration, dynamic_cover, format, has_download_suffix_logo_addr, has_watermark,
                                                height, width, horizontal_type, is_h265, is_long_video, is_source_HDR, meta, misc_download_addrs, origin_cover,
                                                play_addr, play_addr_265, play_addr_h264, ratio, video_model)
};

struct CommentPermissionInfo
{
    bool can_comment{};
    int comment_permission_status{};
    bool item_detail_entry{};
    bool press_entry{};
    bool toast_guide{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CommentPermissionInfo, can_comment, comment_permission_status, item_detail_entry, press_entry, toast_guide)
};

struct CommonFlags
{
    std::string api_ab;
    std::string item_author_nickname;
    std::string video_labels_v2_tag1;
    std::string video_labels_v2_tag2;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CommonFlags, api_ab, item_author_nickname, video_labels_v2_tag1, video_labels_v2_tag2)
};

struct FeedCommentConfig
{
    int audio_comment_permission{};
    int author_audit_status{};
    int common_comment_permission{};
    CommonFlags common_flags{};
    int double_publish_limit{};
    std::string input_config_text;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FeedCommentConfig, audio_comment_permission, author_audit_status, common_comment_permission, common_flags,
                                                double_publish_limit, input_config_text)
};

struct RiskInfos
{
    std::string content;
    std::string icon_url;
    bool risk_sink{};
    int type{};
    bool vote{};
    bool warn{};
    int warn_level{};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(RiskInfos, content, icon_url, risk_sink, type, vote, warn, warn_level)
};

struct Statistics
{
    int64_t admire_count{};
    std::string aweme_id;
    int64_t collect_count{};
    int64_t comment_count{};
    int64_t digg_count{};
    int64_t play_count{};
    int64_t recommend_count{};
    int64_t share_count{};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Statistics, admire_count, aweme_id, collect_count, comment_count, digg_count, play_count, recommend_count,
                                                share_count)
};

struct ReviewResult
{
    int review_status{};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ReviewResult, review_status)
};

struct Status
{
    bool allow_friend_recommend{};
    bool allow_friend_recommend_guide{};
    bool allow_self_recommend_to_friend{};
    bool allow_share{};
    std::string aweme_id;
    int enable_soft_delete{};
    bool in_reviewing{};
    bool is_delete{};
    bool is_prohibited{};
    int listen_video_status{};
    std::string not_allow_soft_del_reason;
    int part_see{};
    int private_status{};
    ReviewResult review_result;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Status, allow_friend_recommend, allow_friend_recommend_guide, allow_self_recommend_to_friend, allow_share,
                                                aweme_id, enable_soft_delete, in_reviewing, is_delete, is_prohibited, listen_video_status,
                                                not_allow_soft_del_reason, part_see, private_status, review_result)
};

struct SuggestWordItem
{
    std::string info;
    std::string word;
    std::string word_id;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SuggestWordItem, info, word, word_id)
};

struct SuggestWordsGroup
{
    std::string extra_info;
    std::string hint_text;
    std::string icon_url;
    std::string scene;
    std::vector<SuggestWordItem> words;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SuggestWordsGroup, extra_info, hint_text, icon_url, scene, words)
};

struct SuggestWords
{
    std::vector<SuggestWordsGroup> suggest_words;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SuggestWords, suggest_words)
};

struct FailInfo
{
    int code{};
    std::string reason;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FailInfo, code, reason)
};

struct ControlInfo
{
    FailInfo fail_info;
    int level{};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ControlInfo, fail_info, level)
};

struct VideoControl
{
    bool allow_douplus{};
    bool allow_download{};
    bool allow_duet{};
    bool allow_dynamic_wallpaper{};
    bool allow_music{};
    bool allow_react{};
    bool allow_record{};
    bool allow_share{};
    bool allow_stitch{};
    std::string disable_record_reason;
    bool download_ignore_visibility{};
    ControlInfo download_info;
    int draft_progress_bar{};
    bool duet_ignore_visibility{};
    ControlInfo duet_info;
    int prevent_download_type{};
    bool share_grayed{};
    bool share_ignore_visibility{};
    int share_type{};
    int show_progress_bar{};
    int timer_status{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(VideoControl, allow_douplus, allow_download, allow_duet, allow_dynamic_wallpaper, allow_music, allow_react,
                                                allow_record, allow_share, allow_stitch, disable_record_reason, download_ignore_visibility, download_info,
                                                draft_progress_bar, duet_ignore_visibility, duet_info, prevent_download_type, share_grayed,
                                                share_ignore_visibility, share_type, show_progress_bar, timer_status)
};

struct VideoTag
{
    int level{};
    int64_t tag_id{};
    std::string tag_name;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(VideoTag, level, tag_id, tag_name)
};

struct VisualSearchInfo
{
    bool is_ecom_img{};
    bool is_high_accuracy_ecom{};
    bool is_high_recall_ecom{};
    bool is_show_img_entrance{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(VisualSearchInfo, is_ecom_img, is_high_accuracy_ecom, is_high_recall_ecom, is_show_img_entrance)
};

struct XiguaBaseInfo
{
    int64_t item_id{};
    int64_t star_altar_order_id{};
    int star_altar_type{};
    int status{};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(XiguaBaseInfo, item_id, star_altar_order_id, star_altar_type, status)
};

struct RightsInfo
{
    bool has_paid{};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(RightsInfo, has_paid)
};

struct SeriesContentType
{
    std::string name;
    int series_content_type{};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SeriesContentType, name, series_content_type)
};

struct SeriesInfo
{
    Author author;
    int content_sub_type{};
    int64_t create_time{};

    UrlModel dark_icon_url;
    std::string desc;
    UrlModel horizontal_cover_url;
    int is_charge_series{};
    int is_exclusive{};
    int is_iaa{};
    UrlModel light_icon_url;
    RightsInfo rights_info;
    std::string real_name;

    std::vector<SeriesContentType> series_content_types;

    int series_form_type{};
    std::string series_id;
    std::string series_name;
    int series_price{};
    int series_type{};

    ShareInfo share_info;

    Statistics statistics{};
    Status status{};
    int64_t update_time{};
    int watched_episode{};
    std::string watched_item;
    int64_t watched_time{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SeriesInfo, author, content_sub_type, create_time, dark_icon_url, desc, horizontal_cover_url, is_charge_series,
                                                is_exclusive, is_iaa, light_icon_url, rights_info, real_name, series_content_types, series_form_type, series_id,
                                                series_name, series_price, series_type, share_info, statistics, status, update_time, watched_episode,
                                                watched_item, watched_time)
};

struct AwemeDetail
{
    int activity_video_type{};
    std::string authentication_token;

    Author author;
    int author_mask_tag{};
    int64_t author_user_id{};

    AwemeControl aweme_control;
    std::string aweme_id;

    int aweme_type{};
    std::string aweme_type_tags{};
    int boost_status{};
    bool can_cache_to_local{};

    std::string caption;
    int cf_assets_type{};
    int cf_recheck_ts{};

    int collect_stat{};
    int collection_corner_mark{};

    int64_t comment_gid{};
    CommentPermissionInfo comment_permission_info;
    std::string component_info_v2{};

    int create_time{};
    std::string desc{};

    int douplus_user_type{};
    std::string douyin_p_c_video_extra;
    std::string douyin_pc_video_extra_seo;
    bool duet_aggregate_in_music_tab{};

    int duration{};
    FeedCommentConfig feed_comment_config;

    int is_24_story{};
    int is_25_story{};
    bool is_ads{};
    int is_collects_selected{};
    bool is_duet_sing{};
    bool is_from_ad_auth{};
    bool is_image_beat{};
    bool is_life_item{};
    int is_moment_history{};
    int is_moment_story{};
    int is_new_text_mode{};
    bool is_share_post{};
    int is_story{};
    int is_top{};
    bool is_use_music{};
    int item_aigc_follow_shot{};
    std::string item_title{};
    bool mark_largely_following{};
    int media_type{};
    Music music;
    std::string pack_usage_scene_by_req_path;
    bool pc_need_login{};
    int rate{};

    RiskInfos risk_infos;

    SeriesInfo series_info;

    Statistics statistics;
    Status status;

    SuggestWords suggest_words;
    int user_digged{};
    int user_recommend_status{};

    Video video;
    VideoControl video_control;
    std::vector<VideoTag> video_tag;

    XiguaBaseInfo xigua_base_info;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AwemeDetail, activity_video_type, author, author_mask_tag, author_user_id, aweme_control, aweme_id, aweme_type,
                                                aweme_type_tags, boost_status, can_cache_to_local, caption, cf_assets_type, cf_recheck_ts, collect_stat,
                                                collection_corner_mark, comment_gid, comment_permission_info, component_info_v2, create_time, desc,
                                                douplus_user_type, douyin_p_c_video_extra, douyin_pc_video_extra_seo, duet_aggregate_in_music_tab, duration,
                                                feed_comment_config, is_24_story, is_25_story, is_ads, is_collects_selected, is_duet_sing, is_from_ad_auth,
                                                is_image_beat, is_life_item, is_moment_history, is_moment_story, is_new_text_mode, is_share_post, is_story,
                                                is_top, is_use_music, item_aigc_follow_shot, item_title, mark_largely_following, media_type, music,
                                                pack_usage_scene_by_req_path, pc_need_login, rate, risk_infos, series_info, statistics, status, suggest_words,
                                                user_digged, user_recommend_status, video, video_control, video_tag, xigua_base_info)
};

struct SeriesDetail
{
    std::vector<AwemeDetail> aweme_list;
    bool has_more{};
    int cursor{};
    int max_cursor{};
    int min_cursor{};
    int status_code{};
    std::string status_msg;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SeriesDetail, aweme_list, has_more, max_cursor, min_cursor, status_code, status_msg)
};

struct AwemeDetailResponse
{
    AwemeDetail aweme_detail;
    int status_code{};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AwemeDetailResponse, aweme_detail, status_code)
};

struct FollowingUser
{
    std::string account_cert_info;
    std::string avatar_signature;
    UrlModel avatar_small;
    UrlModel avatar_thumb;

    int birthday_hide_level{};
    int bless_end_time{};
    int commerce_user_level{};

    std::string custom_verify;
    std::string enterprise_verify_reason;

    int follow_status{};
    int follower_status{};
    bool has_e_account_role{};
    int im_activeness{};
    std::vector<int> im_role_ids;

    int is_im_oversea_user{};
    std::string nickname;
    std::string sec_uid;
    std::string short_id;
    std::string signature;

    int social_relation_sub_type{};
    int social_relation_type{};

    std::string store_region;
    std::string uid;
    std::string unique_id;

    int verification_type{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FollowingUser, account_cert_info, avatar_signature, avatar_small, avatar_thumb, birthday_hide_level,
                                                bless_end_time, commerce_user_level, custom_verify, enterprise_verify_reason, follow_status, follower_status,
                                                has_e_account_role, im_activeness, im_role_ids, is_im_oversea_user, nickname, sec_uid, short_id, signature,
                                                social_relation_sub_type, social_relation_type, store_region, uid, unique_id, verification_type)
};

struct SortedInfo
{
    int64_t conv_short_id{};
    std::string conv_short_id_str;
    int conv_type{};
    std::string sec_uid;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SortedInfo, conv_short_id, conv_short_id_str, conv_type, sec_uid)
};

struct FollowingResponse
{
    int abnormal_sorted{};

    std::vector<FollowingUser> followings;

    int has_more{};
    std::string hotsoon_hide_en_text;
    std::string hotsoon_hide_text;

    int64_t max_time{};
    int64_t min_time{};

    bool mix_recent_share_permenent{};

    int next_req_count{};
    std::string owner_sec_uid;

    std::vector<SortedInfo> sorted_info;

    int status_code{};
    bool update_remove_share_panel{};
    bool with_recent_share_day{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FollowingResponse, abnormal_sorted, followings, has_more, hotsoon_hide_en_text, hotsoon_hide_text, max_time,
                                                min_time, mix_recent_share_permenent, next_req_count, owner_sec_uid, sorted_info, status_code,
                                                update_remove_share_panel, with_recent_share_day)
};

struct WebIdResponse
{
    std::string web_id;
    int e{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WebIdResponse, web_id, e)
};

}  // namespace douyinapi
