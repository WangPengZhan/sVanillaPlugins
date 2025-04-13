#include "CipherList.h"

namespace youtubeapi
{

std::string ReverseCipher::decipher(const std::string& input)
{
    std::string result = input;
    std::reverse(result.begin(), result.end());
    return result;
}

std::string ReverseCipher::to_string()
{
    return "Reverse";
}

SpliceCipher::SpliceCipher(int index)
    : AbstractCipher()
    , m_index(index)
{
}

std::string SpliceCipher::decipher(const std::string& input)
{
    return input.substr(index());
}

std::string SpliceCipher::to_string()
{
    return "Splice " + std::to_string(index());
}

int SpliceCipher::index() const
{
    return m_index;
}

void SpliceCipher::setIndex(int index)
{
    m_index = index;
}

SwapCipher::SwapCipher(int index)
    : AbstractCipher()
    , m_index(index)
{
}

std::string SwapCipher::decipher(const std::string& input)
{
    int i = index();
    if (i >= input.size() || index() < 0)
    {
        return input;
    }
      
    std::string result = input;
    std::swap(result[0], result[i]);
    return result;
}

std::string SwapCipher::to_string()
{
    return "Swap " + std::to_string(index());
}

int SwapCipher::index() const
{
    return m_index;
}

void SwapCipher::setIndex(int index)
{
    m_index = index;
}

}  // namespace youtubeapi
