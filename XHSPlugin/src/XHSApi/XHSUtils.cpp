#include "XHSUtils.h"

#include <random>
#include <regex>

#include <PluginCrypto/Crc32.h>
#include <PluginCrypto/Crypto.h>
#include <PluginCrypto/Encoding.h>
#include <Util/LocaleHelper.h>

namespace xhsapi
{
std::string getCurrentTimeHexMs()
{
    auto now = std::chrono::system_clock::now();
    auto ms = duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    std::stringstream ss;
    ss << std::hex << ms;
    return ss.str();
}

std::string generateRandomString(size_t length, const char* chars)
{
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<> dist(0, strlen(chars) - 1);

    std::string result;
    result.reserve(length);
    for (size_t i = 0; i < length; ++i)
    {
        result += chars[dist(rng)];
    }
    return result;
}

std::pair<std::string, std::string> generateCookieA1AndWebId()
{
    static constexpr char chars[] = "abcdef0123456789";
    static constexpr size_t traceIdLength = 30;
    static constexpr int GET_PLAT_FROM_CODE = 5;

    std::string hexData = getCurrentTimeHexMs();
    std::string randomString = generateRandomString(traceIdLength, chars);
    std::string text = hexData + randomString + "0000";

    uint32_t crc = checksum::crc32(text);

    std::string a1 = text + std::to_string(crc);
    if (a1.size() > 52)
    {
        a1 = a1.substr(0, 52);
    }

    std::string webId = crypto::md5Raw(a1);
    return {a1, webId};
}

std::pair<std::string, std::string> searchJsvmpData(const std::string& data)
{
    std::regex bRegex(R"("b":"(.*?)\",)");
    std::regex dRegex(R"("d":(.*?)\)})");
    std::smatch match;

    std::string b;
    if (std::regex_search(data, match, bRegex) && match.size() > 1)
    {
        b = match[1].str();
    }

    std::string d;
    if (std::regex_search(data, match, dRegex) && match.size() > 1)
    {
        d = match[1].str();
    }
    return {b, d};
}

std::vector<std::vector<int>> decodeJsvmpToLogicList(const std::string& encoded_str)
{
    std::string padded = encoded_str;
    size_t padding = padded.length() % 4;
    if (padding)
    {
        padded.append(4 - padding, '=');
    }
    std::string decoded_bytes = encoding::base64Decode(padded);
    std::vector<uint32_t> codepoints = encoding::utf8ToCodePoints(decoded_bytes);

    std::vector<std::vector<int>> result;
    std::vector<int> current_chunk;
    for (uint32_t cp : codepoints)
    {
        int char_code = static_cast<int>(cp);
        current_chunk.push_back(char_code - 1);
        if (current_chunk.size() == 5)
        {
            result.push_back(std::move(current_chunk));
            current_chunk.clear();
        }
    }
    if (!current_chunk.empty())
    {
        result.push_back(std::move(current_chunk));
    }
    return result;
}

std::string genWebsection(const std::string& data)
{
    auto [b, d] = searchJsvmpData(data);
    std::vector<std::vector<int>> logicList = decodeJsvmpToLogicList(b);

    nlohmann::json dArr = nlohmann::json::parse(d);
    int start = dArr[92].get<int>();
    int end = dArr[93].get<int>();
    if (start < 0 || end >= static_cast<int>(logicList.size()) || start > end)
    {
        return {};
    }

    std::vector<std::vector<int>> targetList = std::vector<std::vector<int>>(logicList.begin() + start, logicList.begin() + end + 1);

    std::vector<int> key(64);
    for (int i = 0; i < 128; i += 2)
    {
        int targetIndex = 675 + i;
        if (targetIndex >= static_cast<int>(targetList.size()))
        {
            return {};
        }
        int dIndex = targetList[targetIndex][2];
        key[i / 2] = dArr[dIndex].get<int>();
    }

    std::string decodeKey;
    decodeKey.reserve(64);
    for (int i = 56; i >= 0; i -= 8)
    {
        for (int j = 0; j < 8; ++j)
        {
            int chCode = key[i + j];
            decodeKey.push_back(static_cast<char>(chCode));
        }
    }
    return decodeKey;
}

}  // namespace xhsapi
