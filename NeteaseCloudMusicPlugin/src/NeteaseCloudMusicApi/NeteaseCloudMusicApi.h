#pragma once

#include <nlohmann/json.hpp>
#include <list>
#include <string>
#include <vector>

namespace netease
{
enum class CryptoType
{
    WeApi,
    EApi,
    LinuxApi,
    Api,
    Unknown,
};

enum class OsType
{
    Pc,
    Linux,
    Android,
    Ios,
    Unknown,
};

enum class LoginType
{
    QRCode,
    Password,
};

enum class SoundLevel
{
    Standard,
    Higher,
    Exhigh,
    Lossless,
    Hires,
    Jyeffect,
    Sky,
    Jymaster,
};

inline std::string soundLevelToString(SoundLevel level)
{
    switch (level)
    {
    case SoundLevel::Standard:
        return "standard";
    case SoundLevel::Higher:
        return "higher";
    case SoundLevel::Exhigh:
        return "exhigh";
    case SoundLevel::Lossless:
        return "lossless";
    case SoundLevel::Hires:
        return "hires";
    case SoundLevel::Jyeffect:
        return "jyeffect";
    case SoundLevel::Sky:
        return "sky";
    case SoundLevel::Jymaster:
        return "jymaster";
    default:
        return "standard";
    }
}

enum class IDType
{
    Song,
    Album,
    Artist,
    Playlist,
    MV,
    User,
    Comment,
    Unknown,
};

inline std::string idTypeToString(IDType type)
{
    switch (type)
    {
    case IDType::Song:
        return "song";
    case IDType::Album:
        return "album";
    case IDType::Artist:
        return "artist";
    case IDType::Playlist:
        return "playlist";
    case IDType::MV:
        return "mv";
    case IDType::User:
        return "user";
    case IDType::Comment:
        return "comment";
    case IDType::Unknown:
    default:
        return "unknown";
    }
}

inline IDType stringToIDType(const std::string& type)
{
    if (type == "song")
        return IDType::Song;
    if (type == "album")
        return IDType::Album;
    if (type == "artist")
        return IDType::Artist;
    if (type == "playlist")
        return IDType::Playlist;
    if (type == "mv")
        return IDType::MV;
    if (type == "user")
        return IDType::User;
    if (type == "comment")
        return IDType::Comment;
    return IDType::Unknown;
}

class LoginUnikey
{
public:
    int code{};
    std::string unikey;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LoginUnikey, code, unikey)
};

struct LoginStatus
{
    int code{};
    std::string message;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LoginStatus, code, message)
};

struct AlbumRef
{
    std::string name;
    int id;
    std::string picUrl;
    std::string pic_str;
    int pic;
    std::vector<std::string> alias;
    std::vector<std::string> artists;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AlbumRef, name, id, picUrl, pic_str, pic, alias, artists)
};

struct ArtistRef
{
    int id;
    std::string name;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ArtistRef, id, name)
};

struct AudioQuality
{
    int br;
    int fid;
    int size;
    double vd;
    int sr;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AudioQuality, br, fid, size, vd, sr)
};

struct Song
{
    std::string name;
    uint64_t id;
    uint64_t pst;
    uint64_t t;
    double pop;
    int st;
    std::string rt;
    std::vector<ArtistRef> ar;
    int fee;
    int v;
    std::string cf;
    AlbumRef al;
    int dt;
    AudioQuality h;
    AudioQuality m;
    AudioQuality l;
    AudioQuality sq;
    std::string cd;
    int no;
    int ftype;
    int djId;
    int copyright;
    int s_id;
    long long mark;
    int originCoverType;
    bool resourceState;
    int version;
    int single;
    int mv;
    int rtype;
    int mst;
    int cp;
    long long publishTime;
    std::vector<std::string> tns;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Song, name, id, pst, t, pop, st, rt, ar, fee, v, cf, al, dt, h, m, l, sq, cd, no, ftype, djId, copyright, s_id,
                                                mark, originCoverType, resourceState, version, single, mv, rtype, mst, cp, publishTime, tns)
};

struct FreeTrialPrivilege
{
    bool resConsumable;
    bool userConsumable;
    std::string listenType;
    std::string cannotListenReason;
    std::string playReason;
    std::string freeLimitTagType;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FreeTrialPrivilege, resConsumable, userConsumable, listenType, cannotListenReason, playReason, freeLimitTagType)
};

struct ChargeInfo
{
    int rate;
    std::string chargeUrl;
    std::string chargeMessage;
    int chargeType;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ChargeInfo, rate, chargeUrl, chargeMessage, chargeType)
};

struct SongPrivilege
{
    uint64_t id;
    int fee;
    int payed;
    int st;
    int pl;
    int dl;
    int sp;
    int cp;
    int subp;
    bool cs;
    int maxbr;
    int fl;
    bool toast;
    int flag;
    bool preSell;
    int playMaxbr;
    int downloadMaxbr;
    std::string maxBrLevel;
    std::string playMaxBrLevel;
    std::string downloadMaxBrLevel;
    std::string plLevel;
    std::string dlLevel;
    std::string flLevel;
    int rscl;
    FreeTrialPrivilege freeTrialPrivilege;
    int rightSource;
    std::vector<ChargeInfo> chargeInfoList;
    int code;
    std::string message;
    std::string plLevels;
    std::string dlLevels;
    std::string ignoreCache;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SongPrivilege, id, fee, payed, st, pl, dl, sp, cp, subp, cs, maxbr, fl, toast, flag, preSell, playMaxbr,
                                                downloadMaxbr, maxBrLevel, playMaxBrLevel, downloadMaxBrLevel, plLevel, dlLevel, flLevel, rscl,
                                                freeTrialPrivilege, rightSource, chargeInfoList, code, message, plLevels, dlLevels, ignoreCache)
};

struct SongDetails
{
    std::vector<Song> songs;
    std::vector<SongPrivilege> privileges;
    int code;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SongDetails, songs, privileges, code)
};

struct FreeTimeTrialPrivilege
{
    bool resConsumable = false;
    bool userConsumable = false;
    int type = 0;
    int remainTime = 0;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FreeTimeTrialPrivilege, resConsumable, userConsumable, type, remainTime)
};

struct AlgData
{
    std::string fragSource;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AlgData, fragSource)
};

struct FreeTrialInfo
{
    int fragmentType = 0;
    int start = 0;
    int end = 0;
    AlgData algData;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FreeTrialInfo, fragmentType, start, end, algData)
};

struct SongPlayUrlInfo
{
    uint64_t id = 0;
    std::string url;
    int br = 0;
    int size = 0;
    std::string md5;
    int code = 0;
    int expi = 0;
    std::string type;
    double gain = 0.0;
    double peak = 0.0;
    double closedGain = 0.0;
    double closedPeak = 0.0;
    int fee = 0;
    std::string uf;
    int payed = 0;
    int flag = 0;
    bool canExtend = false;
    FreeTrialInfo freeTrialInfo;
    std::string level;
    std::string encodeType;
    std::string channelLayout;
    FreeTrialPrivilege freeTrialPrivilege;
    FreeTimeTrialPrivilege freeTimeTrialPrivilege;
    int urlSource = 0;
    int rightSource = 0;
    std::string podcastCtrp;
    std::string effectTypes;
    int time = 0;
    std::string message;
    std::string levelConfuse;
    std::string musicId;
    std::string accompany;
    int sr = 0;
    std::string auEff;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SongPlayUrlInfo, id, url, br, size, md5, code, expi, type, gain, peak, closedGain, closedPeak, fee, uf, payed,
                                                flag, canExtend, freeTrialInfo, level, encodeType, channelLayout, freeTrialPrivilege, freeTimeTrialPrivilege,
                                                urlSource, rightSource, podcastCtrp, effectTypes, time, message, levelConfuse, musicId, accompany, sr, auEff)
};

struct SongPlayUrl
{
    std::vector<SongPlayUrlInfo> data;
    int code;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SongPlayUrl, data, code)
};

struct Album
{
    uint64_t id;
    std::string name;
    std::string type;
    std::string subType;
    std::string description;
    std::string briefDesc;

    int64_t publishTime;

    std::string company;
    int companyId;
    int copyrightId;

    bool paid;
    bool onSale;

    int mark;
    int status;
    int size;

    std::string picUrl;
    std::string blurPicUrl;
    std::string picId_str;
    int64_t picId;
    int64_t pic;

    std::string commentThreadId;

    std::string tags;
    std::vector<std::string> alias;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Album, id, name, type, subType, description, briefDesc, publishTime, company, companyId, copyrightId, paid,
                                                onSale, mark, status, size, picUrl, blurPicUrl, picId_str, picId, pic, commentThreadId, tags, alias)
};

struct AblumDetails
{
    Album album;
    std::vector<Song> songs;
    bool resourceState;
    int code;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AblumDetails, album, songs, resourceState, code)
};

struct TrackId
{
    uint64_t id;
    int v;
    int t;
    uint64_t at;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(TrackId, id, v, t, at)
};

struct Playlist
{
    int64_t id;
    std::string name;

    int64_t coverImgId;
    std::string coverImgUrl;
    std::string coverImgId_str;

    int adType;
    int64_t userId;

    int64_t createTime;
    int64_t updateTime;
    int64_t trackUpdateTime;
    int64_t trackNumberUpdateTime;

    int status;
    int specialType;
    int privacy;

    bool opRecommend;
    bool highQuality;
    bool newImported;
    bool ordered;

    int trackCount;
    int playCount;
    int subscribedCount;
    int cloudTrackCount;

    std::string commentThreadId;
    std::string description;

    std::vector<TrackId> trackIds;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Playlist, id, name, coverImgId, coverImgUrl, coverImgId_str, adType, userId, createTime, updateTime,
                                                trackUpdateTime, trackNumberUpdateTime, status, specialType, privacy, opRecommend, highQuality, newImported,
                                                ordered, trackCount, playCount, subscribedCount, cloudTrackCount, commentThreadId, description, trackIds)
};

struct PlaylistDetails
{
    Playlist playlist;
    int code;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlaylistDetails, playlist, code)
};

struct Bitrate
{
    double size;
    int br;
    int point;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Bitrate, size, br, point)
};

struct MVData
{
    int64_t id;
    std::string name;
    int64_t artistId;
    std::string artistName;
    std::string briefDesc;
    std::string desc;
    std::string cover;
    std::string coverId_str;
    int64_t coverId;
    int playCount;
    int subCount;
    int shareCount;
    int commentCount;
    int duration;
    int nType;
    std::string publishTime;
    std::vector<Bitrate> brs;
    std::vector<ArtistRef> artists;
    std::string commentThreadId;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MVData, id, name, artistId, artistName, briefDesc, desc, cover, coverId_str, coverId, playCount, subCount,
                                                shareCount, commentCount, duration, nType, publishTime, brs, artists, commentThreadId)
};

struct MVResponse
{
    std::string loadingPic;
    std::string bufferPic;
    std::string loadingPicFS;
    std::string bufferPicFS;
    bool subed;
    MVData data;
    int code;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MVResponse, loadingPic, bufferPic, loadingPicFS, bufferPicFS, subed, data, code)
};

struct MVUrlData
{
    int64_t id;
    std::string url;
    int r;
    int64_t size;
    std::string md5;
    int code;
    int expi;
    int fee;
    int mvFee;
    int st;
    std::string msg;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MVUrlData, id, url, r, size, md5, code, expi, fee, mvFee, st, msg)
};

struct MVPlayUrl
{
    int code;
    MVUrlData data;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MVPlayUrl, code, data)
};

}  // namespace netease
