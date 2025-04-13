#include "CipherContainer.h"

namespace youtubeapi
{
std::string CipherContainer::decipher(const std::string& input)
{
    std::string result = input;
    for (const auto& cipher : m_ciphers)
    {
        if (cipher)
        {
            result = cipher->decipher(result);
        }
    }
    return result;
}

std::string CipherContainer::to_string()
{
    std::string result;
    for (const auto& cipher : m_ciphers)
    {
        if (cipher)
        {
            result += cipher->to_string() + "|";
        }
    }
    return result;
}

void CipherContainer::addCipher(std::shared_ptr<AbstractCipher> cipher)
{
    if (cipher)
    {
        m_ciphers.push_back(cipher);
    }
}

void CipherContainer::removeCipher(std::shared_ptr<AbstractCipher> cipher)
{
    m_ciphers.remove(cipher);
}

}  // namespace youtubeapi
