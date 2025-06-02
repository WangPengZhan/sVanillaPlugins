#pragma once
#include <string>

namespace hlsapi
{
const std::string ext_m3u = "#EXTM3U";
const std::string ext_x_targetduration = "#EXT-X-TARGETDURATION";
const std::string ext_x_media_sequence = "#EXT-X-MEDIA-SEQUENCE";
const std::string ext_x_discontinuity_sequence = "#EXT-X-DISCONTINUITY-SEQUENCE";
const std::string ext_x_program_date_time = "#EXT-X-PROGRAM-DATE-TIME";
const std::string ext_x_media = "#EXT-X-MEDIA";
const std::string ext_x_playlist_type = "#EXT-X-PLAYLIST-TYPE";
const std::string ext_x_key = "#EXT-X-KEY";
const std::string ext_x_stream_inf = "#EXT-X-STREAM-INF";
const std::string ext_x_version = "#EXT-X-VERSION";
const std::string ext_x_allow_cache = "#EXT-X-ALLOW-CACHE";
const std::string ext_x_endlist = "#EXT-X-ENDLIST";
const std::string extinf = "#EXTINF";
const std::string ext_i_frames_only = "#EXT-X-I-FRAMES-ONLY";
const std::string ext_x_byterange = "#EXT-X-BYTERANGE";
const std::string ext_x_i_frame_stream_inf = "#EXT-X-I-FRAME-STREAM-INF";
const std::string ext_x_discontinuity = "#EXT-X-DISCONTINUITY";
const std::string ext_x_cue_out_start = "#EXT-X-CUE-OUT";
const std::string ext_x_cue_out = "#EXT-X-CUE-OUT-CONT";
const std::string ext_is_independent_segments = "#EXT-X-INDEPENDENT-SEGMENTS";
const std::string ext_x_scte35 = "#EXT-OATCLS-SCTE35";
const std::string ext_x_cue_start = "#EXT-X-CUE-OUT";
const std::string ext_x_cue_end = "#EXT-X-CUE-IN";
const std::string ext_x_cue_span = "#EXT-X-CUE-SPAN";
const std::string ext_x_map = "#EXT-X-MAP";
const std::string ext_x_start = "#EXT-X-START";

const std::string str_none = "NONE";

const std::string hdcp_level_type_0 = "TYPE-0";
const std::string hdcp_level_type_1 = "TYPE-1";
const std::string hdcp_level_none = str_none;

const std::string video_range_sdr = "SDR";
const std::string video_range_hlg = "HLG";
const std::string video_range_pq = "PQ";

const std::string video_channel_shereo = "CH-STEREO";
const std::string video_channel_mono = "CH-MONO";

const std::string media_type_audio = "AUDIO";
const std::string media_type_video = "VIDEO";
const std::string media_type_subtitle = "SUBTITLES";
const std::string media_type_closed_caption = "CLOSED-CAPTIONS";

const std::string key_method_aes128 = "AES-128";
const std::string key_method_sample_aes = "SAMPLE-AES";
const std::string key_method_none = str_none;
const std::string key_method_sample_aes_ctr = "SAMPLE-AES-CTR";

}  // namespace hlsapi
