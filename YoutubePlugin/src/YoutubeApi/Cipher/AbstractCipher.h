#pragma once
#include <string>

namespace youtubeapi
{

class AbstractCipher
{
public:
    virtual ~AbstractCipher() = default;
    virtual std::string decipher(const std::string& input) = 0;
    virtual std::string to_string() = 0;
};

}  // namespace youtubeapi
