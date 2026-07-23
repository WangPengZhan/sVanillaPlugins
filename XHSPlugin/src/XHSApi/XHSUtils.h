#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <nlohmann/json.hpp>

namespace xhsapi
{
std::pair<std::string, std::string> generateCookieA1AndWebId();
std::pair<std::string, std::string> searchJsvmpData(const std::string& data);
std::string genWebsection(const std::string& data);

enum class SignFormat
{
    Xys,
    Xyw,
};

struct SignatureHeaders
{
    std::string xS;
    std::string xSCommon;
    std::string xT;
};

using SignCookies = std::map<std::string, std::string>;
SignCookies parseSignCookies(const std::string& cookieHeader);
SignatureHeaders signRequest(const std::string& method, const std::string& uri, const std::string& body, const SignCookies& cookies, SignFormat format,
                             std::int64_t timestampMs = 0);

}  // namespace xhsapi
