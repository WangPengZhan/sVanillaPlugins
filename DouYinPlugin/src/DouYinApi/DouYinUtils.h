#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace douyinapi
{

class ABogus
{
public:
    explicit ABogus(std::string userAgent, std::string platform = "Win32");

    std::string getValue(const std::string& query, const std::string& body = {}, uint64_t startTime = 0, uint64_t endTime = 0,
                         const std::array<double, 3>& randomValues = {-1, -1, -1}) const;

private:
    static std::string digestParameter(const std::string& value);
    static std::string encodeUserAgent(const std::string& userAgent);
    static std::string createBrowserFingerprint(const std::string& platform);
    static std::string createRandomPrefix(const std::array<double, 3>& randomValues);
    static std::vector<uint8_t> buildCorePayload(const std::string& queryDigest, const std::string& bodyDigest, const std::string& userAgentDigest,
                                                 uint64_t startTime, uint64_t endTime, std::size_t browserFingerprintSize);
    static std::string encryptPayload(const std::vector<uint8_t>& corePayload, const std::string& browserFingerprint);

private:
    std::string m_userAgentDigest;
    std::string m_browserFingerprint;
};

}  // namespace douyinapi
