#pragma once
#include <string>
#include <list>
#include <memory>

#include "AbstractCipher.h"

namespace youtubeapi
{

class CipherContainer : public AbstractCipher
{
public:
     explicit CipherContainer() = default;
     ~CipherContainer() = default;

     std::string decipher(const std::string& input) override;
     std::string to_string() override;

     void addCipher(std::shared_ptr<AbstractCipher> cipher);
     void removeCipher(std::shared_ptr<AbstractCipher> cipher);

 protected:
     std::list<std::shared_ptr<AbstractCipher>> m_ciphers;
};

}  // namespace youtubeapi
