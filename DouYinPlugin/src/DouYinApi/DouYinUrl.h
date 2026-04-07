#include "DouYinApi.h"
#include <regex>

namespace douyinapi
{

enum class IDType
{
    Unkown,
    AwemeId,
    SeriesId,
    MixId,
    UserId,
};

std::string typeToString(IDType type);
IDType stringToType(const std::string& type);

struct IDInfo
{
    std::string id;
    IDType type{IDType::Unkown};
    std::string parentId;
    IDType parentIdType{IDType::Unkown};

    std::string to_string() const;
};

bool isValidUrl(const std::string& url);
IDInfo getID(const std::string& url);

}  // namespace douyinapi
