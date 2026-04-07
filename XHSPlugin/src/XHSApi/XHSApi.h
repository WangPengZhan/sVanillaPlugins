#pragma once
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace xhsapi
{

template <typename Result>
struct XHSBasicResponse
{
    int code{};
    bool success;
    std::string msg;
    Result data;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(XHSBasicResponse<Result>, code, success, msg, data)
};

struct QRCode
{
    std::string qr_id;
    std::string code;
    std::string url;
    int multi_flag{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(QRCode, qr_id, code, url, multi_flag)
};

using QRCodeResponse = XHSBasicResponse<QRCode>;

struct LoginStatusResult
{
    bool success{};
    int code{};
    std::string message;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LoginStatusResult, success, code, message)
};

struct LoginStatus
{
    int codeStatus{};
    std::string userId;
    int geoZone{};
    LoginStatusResult result;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LoginStatus, codeStatus, userId, geoZone, result)
};

using LoginStatusResponse = XHSBasicResponse<LoginStatus>;

struct AccountInfo
{
    std::string user_id;
    std::string nickname;
    std::string desc;
    int gender{};
    std::string images;
    std::string imageb;
    bool guest{};
    std::string red_id;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AccountInfo, user_id, nickname, desc, gender, images, imageb, guest, red_id)
};

using AccountInfoResponse = XHSBasicResponse<AccountInfo>;

struct InteractInfo
{
    bool liked{};
    std::string liked_count;
    bool collected{};
    std::string collected_count;
    std::string comment_count;
    std::string share_count;
    bool followed{};
    std::string relation;
    std::string sticky;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(InteractInfo, liked, liked_count, collected, collected_count, comment_count, share_count, followed, relation,
                                                sticky)
};

struct ShareInfo
{
    bool un_share{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ShareInfo, un_share)
};

struct User
{
    std::string xsec_token;
    std::string user_id;
    std::string nickname;
    std::string avatar;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(User, xsec_token, user_id, nickname, avatar)
};

struct InfoItem
{
    std::string image_scene;
    std::string url;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(InfoItem, image_scene, url)
};

struct ImageItem
{
    int height{};
    int width{};
    std::string url;
    std::string trace_id;
    std::string url_pre;
    std::string url_default;
    bool live_photo{};
    std::string file_id;
    std::vector<InfoItem> info_list;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ImageItem, height, width, url, trace_id, url_pre, url_default, live_photo, file_id, info_list)
};

struct VideoImage
{
    std::string first_frame_fileid;
    std::string thumbnail_fileid;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(VideoImage, first_frame_fileid, thumbnail_fileid)
};

struct VideoCapa
{
    int duration{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(VideoCapa, duration)
};

struct StreamItem
{
    int audio_duration{};
    int audio_channels{};
    int weight{};
    std::string format;
    std::string master_url;
    std::string quality_type;
    int video_bitrate{};
    int rotate{};
    int duration{};
    std::string video_codec;
    int audio_bitrate{};
    std::string stream_desc;
    int width{};
    int size{};
    int volume{};
    int avg_bitrate{};
    int video_duration{};
    std::string audio_codec;
    int stream_type{};
    int vmaf{};
    int default_stream{};
    int hdr_type{};
    double psnr{};
    int height{};
    int fps{};
    std::vector<std::string> backup_urls;
    int ssim{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(StreamItem, audio_duration, audio_channels, weight, format, master_url, quality_type, video_bitrate, rotate,
                                                duration, video_codec, audio_bitrate, stream_desc, width, size, volume, avg_bitrate, video_duration,
                                                audio_codec, stream_type, vmaf, default_stream, hdr_type, psnr, height, fps, backup_urls, ssim)
};

struct Stream
{
    std::vector<StreamItem> h264;
    std::vector<StreamItem> h265;
    std::vector<StreamItem> h266;
    std::vector<StreamItem> av1;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Stream, h264, h265, h266, av1)
};

struct VideoMeta
{
    int biz_name{};
    std::string biz_id;
    int duration{};
    std::string md5;
    int hdr_type{};
    int drm_type{};
    std::vector<int> stream_types;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(VideoMeta, biz_name, biz_id, duration, md5, hdr_type, drm_type, stream_types)
};

struct Media
{
    long long video_id{};
    VideoMeta video;
    Stream stream;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Media, video_id, video, stream)
};

struct Video
{
    VideoImage image;
    VideoCapa capa;
    Media media;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Video, image, capa, media)
};

struct TagItem
{
    std::string name;
    std::string type;
    std::string id;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(TagItem, name, type, id)
};

struct NoteCard
{
    std::string type;
    InteractInfo interact_info;
    long long time{};
    std::string desc;
    std::vector<std::string> at_user_list;
    ShareInfo share_info;
    User user;
    std::vector<ImageItem> image_list;
    Video video;
    std::vector<TagItem> tag_list;
    std::string note_id;
    std::string title;
    long long last_update_time{};
    std::string ip_location;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NoteCard, type, interact_info, time, desc, at_user_list, share_info, user, image_list, video, tag_list, note_id,
                                                title, last_update_time, ip_location)
};

struct NoteDetail
{
    std::string id;
    std::string model_type;
    NoteCard note_card;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NoteDetail, id, model_type, note_card)
};

using NoteDetailResponse = XHSBasicResponse<NoteDetail>;

struct Cover
{
    std::string trace_id;
    std::vector<TagItem> tag_list;
    std::string url_pre;
    std::string url_default;
    std::string file_id;
    int height{};
    int width{};
    std::string url;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Cover, trace_id, tag_list, url_pre, url_default, file_id, height, width, url)
};

struct NoteItemInfo
{
    std::string xsec_token;
    std::string type;
    std::string display_title;

    User user;
    InteractInfo interact_info;

    Cover cover;

    std::string note_id;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NoteItemInfo, xsec_token, type, display_title, user, interact_info, cover, note_id)
};

struct NoteItemList
{
    std::string cursor;
    std::vector<NoteItemInfo> notes;
    bool has_more{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NoteItemList, cursor, notes, has_more)
};

using NoteItemListResponse = XHSBasicResponse<NoteItemList>;

struct ScriptingData
{
    std::string data;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ScriptingData, data)
};

using ScriptingDataResponse = XHSBasicResponse<ScriptingData>;

}  // namespace xhsapi
