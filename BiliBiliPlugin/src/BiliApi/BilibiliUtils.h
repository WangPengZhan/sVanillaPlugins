#pragma once
#include <string>
#include <nlohmann/json.hpp>

#include "BiliApi.h"

namespace biliapi
{
extern std::string cookieDataDir;
constexpr char cookieDataFileName[] = "sVanilla.data";
inline const std::vector mixinKeyEncTab = {46, 47, 18, 2,  53, 8,  23, 32, 15, 50, 10, 31, 58, 3,  45, 35, 27, 43, 5,  49, 33, 9,
                                           42, 19, 29, 28, 14, 39, 12, 38, 41, 13, 37, 48, 7,  16, 24, 55, 40, 61, 26, 17, 0,  1,
                                           60, 51, 30, 4,  22, 25, 54, 21, 56, 59, 6,  63, 57, 62, 11, 36, 20, 34, 44, 52};
void replaceCharacter(std::string& source, const std::string& from, const std::string& to);
std::string filterCharacters(const std::string& input);
std::string urlEncode(const std::string& decoded);
std::string urlDecode(const std::string& encoded);
std::string GetMixinKey(const std::string& orig);
std::string MD5Hash(const std::string& str);
bool isExpired(const std::time_t& expires);
void setCookieDataDir(const std::string& dir);
std::string cookieDataFilePath();

}  // namespace biliapi
