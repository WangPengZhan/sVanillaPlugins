#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "NetWork/CNetWork.h"
#include "HLSApi.h"
#include "StringAndEffectiveUrl.h"

namespace hlsapi
{

bool decryptSegment(const std::vector<std::uint8_t>& cipherData, const KeyInfo& keyInfo, std::uint64_t mediaSequence, std::vector<std::uint8_t>& plainData);
bool mergeTsFiles(const std::string& dir, const std::vector<std::string>& files, const std::string& output, std::function<void()> onSuccess = {});

class HLSClient : public network::NetWork
{
public:
    HLSClient();
    ~HLSClient() = default;

    static HLSClient& globalClient();

    std::string downloadM3u8(const std::string& url, std::string* effectiveUrl = nullptr);
    bool downloadKey(const std::string& url, std::string& key);
    bool downloadTsToFiles(const std::string& uri, const std::string& filename, const KeyInfo& keyInfo = {}, std::uint64_t mediaSequence = 0,
                           std::size_t byteRangeLength = 0, std::size_t byteRangeOffset = 0);

    void mergeTs(const std::string& dir, const std::string& output);

protected:
    void initDefaultHeaders();
    void initDefaultOptions();
};

}  // namespace hlsapi
