#pragma once

#include <regex>
#include <string>

namespace biliapi
{
enum class IDType
{
    Unkown,
    Aid,
    Bid,
    Cid,
    BangumiSS,
    BangumiEP,
    BangumiMD,
    CheeseSS,
    CheeseEP,
    FavoritesId,
    UserId,
};

std::string typeToString(IDType type);
IDType stringToType(const std::string& type);

struct IDInfo
{
    std::string id;
    IDType type{IDType::Unkown};
    std::string parentId;
    IDType parentType{IDType::Unkown};

    std::string to_string() const;
};

bool isValidUrl(const std::string& url);
IDInfo getID(const std::string& url);
}  // namespace biliapi
