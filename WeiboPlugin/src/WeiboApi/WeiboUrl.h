#pragma once

#include <regex>
#include <string>

namespace weiboapi
{

enum class IDType
{
    Unkown,
    Status,
    TV,
};

std::string typeToString(IDType type);
IDType stringToType(const std::string& type);

struct IDInfo
{
    std::string id;
    IDType type{IDType::Unkown};

    std::string to_string() const;
};

bool isValidUrl(const std::string& url);

IDInfo getID(const std::string& url);

}  // namespace weiboapi
