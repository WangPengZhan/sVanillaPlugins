#pragma once
#include <string>
#include <list>
#include <memory>

#include "AbstractCipher.h"

namespace youtubeapi
{

class ReverseCipher : public AbstractCipher
{
public:
    std::string decipher(const std::string& input) override;
    std::string to_string() override;
};

class SpliceCipher : public AbstractCipher
{
public:
    explicit SpliceCipher() = default;
    explicit SpliceCipher(int index);

    std::string decipher(const std::string& input) override;
    std::string to_string() override;

    int index() const;
    void setIndex(int index);

private:
    int m_index{-1};
};

class SwapCipher : public AbstractCipher
{
public:
    explicit SwapCipher() = default;
    explicit SwapCipher(int index);

    std::string decipher(const std::string& input) override;
    std::string to_string() override;

    int index() const;
    void setIndex(int index);

private:
    int m_index{-1};
};

}  // namespace youtubeapi
