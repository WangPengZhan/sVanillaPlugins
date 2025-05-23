#include <openssl/md5.h>
#include <openssl/evp.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <algorithm>
#include <curl/curl.h>

#include "BiliApi.h"
#include "BilibiliUtils.h"

namespace biliapi
{
std::string cookieDataDir;

void replaceCharacter(std::string& source, const std::string& from, const std::string& to)
{
    std::string newString;
    newString.reserve(source.length());

    std::string::size_type lastPos = 0;
    std::string::size_type findPos;

    while (std::string::npos != (findPos = source.find(from, lastPos)))
    {
        newString.append(source, lastPos, findPos - lastPos);
        newString += to;
        lastPos = findPos + from.length();
    }

    newString += source.substr(lastPos);
    source.swap(newString);
}

std::string filterCharacters(const std::string& input)
{
    const std::string charsToFilter = "!\'()*";
    std::string result = input;

    result.erase(std::remove_if(result.begin(), result.end(),
                                [&charsToFilter](const char& c) {
                                    return charsToFilter.find(c) != std::string::npos;
                                }),
                 result.end());

    return result;
}

std::string urlEncode(const std::string& decoded)
{
    auto* const encodedValue = curl_easy_escape(nullptr, decoded.c_str(), static_cast<int>(decoded.length()));
    std::string result(encodedValue);
    curl_free(encodedValue);
    return result;
}

std::string urlDecode(const std::string& encoded)
{
    int outputLength;
    auto* const decodedValue = curl_easy_unescape(nullptr, encoded.c_str(), static_cast<int>(encoded.length()), &outputLength);
    std::string result(decodedValue, outputLength);
    curl_free(decodedValue);
    return result;
}

std::string GetMixinKey(const std::string& orig)
{
    std::string result;
    for (const int i : mixinKeyEncTab)
    {
        result += orig[i];
    }
    return result.substr(0, 32);
}

std::string MD5Hash(const std::string& str)
{
    unsigned char mdValue[EVP_MAX_MD_SIZE];
    unsigned int mdLen;
    // 创建和初始化摘要上下文
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    const EVP_MD* md = EVP_md5();
    // 初始化摘要操作
    EVP_DigestInit_ex(mdctx, md, nullptr);
    // 提供要进行摘要计算的数据
    EVP_DigestUpdate(mdctx, str.c_str(), str.length());
    // 获取摘要结果
    EVP_DigestFinal_ex(mdctx, mdValue, &mdLen);
    // 清理上下文
    EVP_MD_CTX_free(mdctx);
    // 将摘要结果转换为字符串形式
    std::ostringstream oss;
    for (unsigned int i = 0; i < mdLen; i++)
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(mdValue[i]);
    }

    return oss.str();
}

bool isExpired(const std::time_t& expires)
{
    return expires < std::time(nullptr) / 86400;
}

void setCookieDataDir(const std::string& dir)
{
    cookieDataDir = dir;
}

std::string cookieDataFilePath()
{
    std::string filePath;
    if (cookieDataDir.empty())
    {
        filePath = cookieDataFileName;
    }
    else
    {
        filePath =
            (cookieDataDir.back() == '/' || cookieDataDir.back() == '\\') ? cookieDataDir + cookieDataFileName : cookieDataDir + "/" + cookieDataFileName;
    }

    return filePath;
}

}  // namespace biliapi
