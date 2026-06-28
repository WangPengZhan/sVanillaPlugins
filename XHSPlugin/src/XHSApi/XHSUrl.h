#include <regex>

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

bool isValidUrl(const std::string& url);
IDInfo getID(const std::string& url);

}  // namespace xhsapi
