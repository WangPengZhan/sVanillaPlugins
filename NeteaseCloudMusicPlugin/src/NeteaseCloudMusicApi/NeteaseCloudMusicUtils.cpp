#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <algorithm>
#include <codecvt>
#include <locale>

#include <curl/curl.h>

#include <PluginCrypto/Crypto.h>
#include <PluginCrypto/Encoding.h>

#include "NeteaseCloudMusicUtils.h"

#include "NeteaseCloudMusicApi.h"
#include "NeteaseCloudMusicConstants.h"

namespace
{
std::random_device rd;
std::mt19937 gen(rd());

int getRandomInt(int min, int max)
{
    std::uniform_int_distribution<int> dist(min, max);
    return dist(gen);
}

}  // namespace

namespace netease
{
int generateIPSegment()
{
    return getRandomInt(1, 255);
}

std::string generateRandomChineseIP()
{
    const std::array<std::string, 4> chinaIPPrefixes = {"116.25", "116.76", "116.77", "116.78"};

    // Select a random prefix from the array
    const std::string& randomPrefix = chinaIPPrefixes[getRandomInt(0, chinaIPPrefixes.size() - 1)];

    // Format the IP address using string concatenation
    return randomPrefix + "." + std::to_string(generateIPSegment()) + "." + std::to_string(generateIPSegment());
}

std::string generateRandomString(int length, const std::string& charset)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, charset.size() - 1);

    std::string result;
    for (int i = 0; i < length; ++i)
    {
        result += charset[dis(gen)];
    }
    return result;
}
std::string generateWNMCID()
{
    std::string randomString = generateRandomString(6, "abcdefghijklmnopqrstuvwxyz");
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return randomString + "." + std::to_string(now_ms) + ".01.0";
}

std::string cloudMusicDllEncodeID(const std::string& id)
{
    std::string xoredString;
    for (size_t i = 0; i < id.size(); ++i)
    {
        const char ch = id[i] ^ Encrypt::ID_XOR_KEY_1[i % Encrypt::ID_XOR_KEY_1.size()];
        xoredString += ch;
    }

    // 计算MD5摘要
    const std::string digest = crypto::md5Raw(xoredString);
    // Base64编码
    std::string encoded = encoding::base64Encode(digest);
    return encoded;
}

std::unordered_map<std::string, std::string> cookieToJson(const std::string& cookie)
{
    std::unordered_map<std::string, std::string> result;
    if (cookie.empty())
    {
        return result;
    }

    std::string_view str = cookie;
    size_t pos = 0;

    while (pos < str.size())
    {
        // 找到下一个分号
        const size_t next = str.find(';', pos);
        std::string_view item = str.substr(pos, (next == std::string_view::npos ? str.size() : next) - pos);

        // 找到等号
        const size_t eq = item.find('=');
        const auto trim = [](const std::string_view sv) {
            const auto ws = " \t";
            const size_t start = sv.find_first_not_of(ws);
            if (start == std::string_view::npos)
            {
                return std::string_view();
            }
            const size_t end = sv.find_last_not_of(ws);
            return sv.substr(start, end - start + 1);
        };

        if (eq != std::string_view::npos)
        {
            std::string_view key = trim(item.substr(0, eq));
            std::string_view value = trim(item.substr(eq + 1));
            if (!key.empty())
            {  // 跳过空key
                result.emplace(key, value);
            }
        }

        if (next == std::string_view::npos)
        {
            break;
        }
        pos = next + 1;
    }

    return result;
}

std::string cookieObjToString(const std::unordered_map<std::string, std::string>& cookie)
{
    std::stringstream ss;
    bool first = true;

    for (const auto& [fst, snd] : cookie)
    {
        if (!first)
        {
            ss << "; ";
        }
        ss << encoding::urlEncode(fst) << "=" << encoding::urlEncode(snd);
        first = false;
    }
    return ss.str();
}

std::unordered_map<std::string, std::string> weapi(const nlohmann::ordered_json& object)
{
    const std::string text = object.dump();
    const std::string secretKey = generateRandomString(16, Encrypt::BASE62);

    std::string params =
        crypto::aes128Encrypt(crypto::aes128Encrypt(text, "cbc", Encrypt::PRESET_KEY, Encrypt::IV), "cbc", secretKey, Encrypt::IV);
    std::string reversedKey = secretKey;
    std::ranges::reverse(reversedKey);
    std::string encSecKey = crypto::rsaNoPaddingPublicEncryptHexLower(reversedKey, Encrypt::PUBLIC_KEY);

    return {
        {"params",    params   },
        {"encSecKey", encSecKey}
    };
}

std::unordered_map<std::string, std::string> eapi(const std::string& url, const nlohmann::ordered_json& object)
{
    const std::string text = object.dump();
    const std::string message = "nobody" + url + "use" + text + "md5forencrypt";
    const std::string digest = crypto::md5Hex(message);
    const std::string data = url + "-36cd479b6b5-" + text + "-36cd479b6b5-" + digest;

    return {
        {"params", crypto::aes128Encrypt(data, "ecb", Encrypt::EAPI_KEY, "", "hex")}
    };
}

nlohmann::json eapiResDecrypt(const std::string& encryptedParams)
{
    std::string decryptedData = crypto::aes128EcbDecrypt(encryptedParams, Encrypt::EAPI_KEY, "", "hex");
    return nlohmann::json::parse(decryptedData);
}

std::string concatenateCurlHeader(const std::string& name, const std::string& value)
{
    return name + ": " + value + "\r\n";
}

}  // namespace netease
