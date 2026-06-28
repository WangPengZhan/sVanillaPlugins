#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <algorithm>
#include <PluginCrypto/Encoding.h>

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

std::string GetMixinKey(const std::string& orig)
{
    std::string result;
    for (const int i : mixinKeyEncTab)
    {
        result += orig[i];
    }
    return result.substr(0, 32);
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

std::string hmac_sha256(const std::string& key, const std::string& message)
{
    unsigned char* digest;
    digest = HMAC(EVP_sha256(), key.data(), (int)key.size(), reinterpret_cast<const unsigned char*>(message.data()), (int)message.size(), nullptr, nullptr);

    std::ostringstream oss;
    for (int i = 0; i < 32; i++)
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    }
    return oss.str();
}

}  // namespace biliapi
