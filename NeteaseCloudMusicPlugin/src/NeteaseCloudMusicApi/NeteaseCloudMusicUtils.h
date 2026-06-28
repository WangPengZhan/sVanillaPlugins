#pragma once
#include "NeteaseCloudMusicApi.h"

#include <random>
#include <string>
#include <nlohmann/json.hpp>

namespace netease
{
// Generate a random IP segment (1-255)
int generateIPSegment();

std::string generateRandomChineseIP();

std::string generateRandomString(int length, const std::string& charset);

std::string generateWNMCID();

std::string cloudMusicDllEncodeID(const std::string& id);

std::unordered_map<std::string, std::string> cookieToJson(const std::string& cookie);
std::string cookieObjToString(const std::unordered_map<std::string, std::string>& cookie);

std::unordered_map<std::string, std::string> weapi(const nlohmann::ordered_json& object);

nlohmann::json eapiResDecrypt(const std::string& encryptedParams);
std::unordered_map<std::string, std::string> eapi(const std::string& url, const nlohmann::ordered_json& object);

std::string concatenateCurlHeader(const std::string& name, const std::string& value);

}  // namespace netease
