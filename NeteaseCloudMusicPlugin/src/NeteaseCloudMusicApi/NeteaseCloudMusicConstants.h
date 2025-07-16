#pragma once
#include <string>

namespace netease
{

inline const std::string apiBase = "https://interface.music.163.com";
inline const std::string home = "https://music.163.com";
constexpr char domain[] = ".music.163.com";

namespace Login
{
inline const std::string Anonimous = "/api/register/anonimous";
inline const std::string QRUnikey = "/api/login/qrcode/unikey";
inline const std::string QRCreate = "https://music.163.com/login?codekey=";
inline const std::string QRCheck = "/api/login/qrcode/client/login";
inline const std::string QRRefresh = "/api/login/token/refresh";
inline const std::string SongDetail = "/api/v3/song/detail";
inline const std::string SongUrl = "/api/song/enhance/player/url/v1";
inline const std::string Album = "/api/v1/album";
inline const std::string PlaylistDetail = "/api/v6/playlist/detail";
inline const std::string MVDetail = "/api/v1/mv/detail";
inline const std::string MVUrl = "/api/song/enhance/play/mv/url";

}  // namespace Login

namespace Encrypt
{
inline const std::string IV = "0102030405060708";
inline const std::string PRESET_KEY = "0CoJUm6Qyw8W8jud";
inline const std::string LINUX_API_KEY = "rFgB&h#%2?^eDg:Q";
inline const std::string BASE62 = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
inline const std::string PUBLIC_KEY = "-----BEGIN PUBLIC KEY-----\n"
                                      "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDgtQn2JZ34ZC28NWYpAUd98iZ3\n"
                                      "7BUrX/aKzmFbt7clFSs6sXqHauqKWqdtLkF2KexO40H1YTX8z2lSgBBOAxLsvakl\n"
                                      "V8k4cBFK9snQXE9/DDaFt6Rr7iVZMldczhC0JNgTz+SHXT6CBHuX3e9SdB1Ua44on\n"
                                      "caTWz7OBGLbCiK45wIDAQAB\n"
                                      "-----END PUBLIC KEY-----\n";
inline const std::string EAPI_KEY = "e82ckenh8dichen8";
inline const std::string ID_XOR_KEY_1 = "3go8&$8*3*3h0k(2)2";

}  // namespace Encrypt

}  // namespace netease
