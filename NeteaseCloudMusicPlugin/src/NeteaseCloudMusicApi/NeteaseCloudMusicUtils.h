#pragma once
#include "NeteaseCloudMusicApi.h"

#include <random>
#include <string>
#include <nlohmann/json.hpp>

#include <PluginConfig.h>

namespace netease
{
// Generate a random IP segment (1-255)
int generateIPSegment();

std::string generateRandomChineseIP();

std::string generateRandomString(int length, const std::string& charset);

std::string generateWNMCID();

// Base64 encoding/decoding
std::string PLUGIN_API base64Encode(const std::string& input);
std::string base64Decode(const std::string& input);

// Hex encoding/decoding
std::string hexEncode(const std::string& input);
std::string hexDecode(const std::string& input);

std::string PLUGIN_API aesEncrypt(const std::string& text, const std::string& mode, const std::string& key, const std::string& iv,
                                  const std::string& format = "base64");
std::string PLUGIN_API aesDecrypt(const std::string& ciphertext, const std::string& key, const std::string& iv, const std::string& format = "base64");

// RSA encryption
std::string PLUGIN_API rsaEncrypt(const std::string& str, const std::string& key);

// MD5 hash
std::string md5(const std::string& input);

std::string urlEncode(const std::string& decoded);

std::string urlDecode(const std::string& encoded);

std::string PLUGIN_API cloudMusicDllEncodeID(const std::string& id);

std::unordered_map<std::string, std::string> cookieToJson(const std::string& cookie);
std::string cookieObjToString(const std::unordered_map<std::string, std::string>& cookie);

std::unordered_map<std::string, std::string> PLUGIN_API weapi(const nlohmann::ordered_json& object);

nlohmann::json eapiResDecrypt(const std::string& encryptedParams);
std::unordered_map<std::string, std::string> eapi(const std::string& url, const nlohmann::json& object);

std::string concatenateCurlHeader(const std::string& name, const std::string& value);

}  // namespace netease
