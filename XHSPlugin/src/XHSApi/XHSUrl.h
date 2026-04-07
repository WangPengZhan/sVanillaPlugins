#include <regex>

#include <PluginConfig.h>

#include "XHSApi.h"

namespace xhsapi
{

enum class IDType
{
    Unkown,
    NoteId,
    UserId,
};

std::string typeToString(IDType type);
IDType stringToType(const std::string& type);

struct IDInfo
{
    std::string id;
    IDType type{IDType::Unkown};
    std::string xsecToken;
    std::string parentId;
    IDType parentIdType{IDType::Unkown};
    std::string parentXsecToken;

    std::string to_string() const;
};

bool PLUGIN_API isValidUrl(const std::string& url);
IDInfo PLUGIN_API getID(const std::string& url);

}  // namespace xhsapi
