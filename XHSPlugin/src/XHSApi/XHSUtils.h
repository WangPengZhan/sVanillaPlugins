#pragma once
#include <string>
#include <nlohmann/json.hpp>

namespace xhsapi
{
std::pair<std::string, std::string> generateCookieA1AndWebId();
std::pair<std::string, std::string> searchJsvmpData(const std::string& data);
std::string genWebsection(const std::string& data);

}  // namespace xhsapi
