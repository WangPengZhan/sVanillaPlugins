#pragma once
#include <string>
#include <nlohmann/json.hpp>

namespace xhsapi
{
std::string urlEncode(const std::string& decoded);
std::string urlDecode(const std::string& encoded);

std::string base64Encode(const std::string& input);
std::string base64Decode(const std::string& input);

std::string hexEncode(const std::string& input);
std::string hexDecode(const std::string& input);

std::vector<uint32_t> utf8ToCodePoints(const std::string& str);
std::string codePointsToUtf8(const std::vector<uint32_t>& cps);

std::string aesEncrypt(const std::string& text, const std::string& mode, const std::string& key, const std::string& iv, const std::string& format = "base64");
std::string aesDecrypt(const std::string& ciphertext, const std::string& key, const std::string& iv, const std::string& format = "base64");

std::string rsaEncrypt(const std::string& str, const std::string& key);

std::string md5(const std::string& input);
uint32_t crc32(const std::string& str);

std::pair<std::string, std::string> generateCookieA1AndWebId();
std::pair<std::string, std::string> searchJsvmpData(const std::string& data);
std::string genWebsection(const std::string& data);

}  // namespace xhsapi
