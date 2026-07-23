#include "DouYinUtils.h"

#include <chrono>
#include <random>
#include <sstream>
#include <string_view>

#include "PluginCrypto/Crypto.h"

namespace douyinapi
{
namespace
{
constexpr std::string_view kSalt = "cus";
constexpr char kUserAgentKey[] = "\x00\x01\x0e";
constexpr std::string_view kDefaultBrowserFingerprint = "1536|742|1536|864|0|0|0|0|1536|864|1536|864|1536|742|24|24|Win32";
constexpr std::string_view kUserAgentAlphabet = "ckdp1h4ZKsUB80/Mfvw36XIgR25+WQAlEi7NLboqYTOPuzmFjJnryx9HVGDaStCe";
constexpr std::string_view kABogusAlphabet = "Dkdpgh2ZmsQB80/MfvV36XI1R45-WUAlEixNLwoqYTOPuzKFjJnry79HbGcaStCe";

int randomInt(int min, int max)
{
    static thread_local std::mt19937 generator(std::random_device{}());
    return std::uniform_int_distribution<int>(min, max)(generator);
}

uint64_t currentTimeMillis()
{
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
}

std::string customBase64(const std::string& input, std::string_view alphabet)
{
    std::string output;
    output.reserve((input.size() + 2) / 3 * 4);

    for (size_t index = 0; index < input.size(); index += 3)
    {
        const uint32_t first = static_cast<uint8_t>(input[index]);
        const uint32_t second = index + 1 < input.size() ? static_cast<uint8_t>(input[index + 1]) : 0;
        const uint32_t third = index + 2 < input.size() ? static_cast<uint8_t>(input[index + 2]) : 0;
        const uint32_t value = (first << 16) | (second << 8) | third;

        output.push_back(alphabet[(value >> 18) & 0x3f]);
        output.push_back(alphabet[(value >> 12) & 0x3f]);
        output.push_back(index + 1 < input.size() ? alphabet[(value >> 6) & 0x3f] : '=');
        output.push_back(index + 2 < input.size() ? alphabet[value & 0x3f] : '=');
    }
    return output;
}

uint8_t xorChecksum(const std::vector<uint8_t>& bytes)
{
    uint8_t checksum = 0;
    for (const uint8_t byte : bytes)
    {
        checksum ^= byte;
    }
    return checksum;
}

void appendMaskedRandom(std::string& output, double suppliedValue, int lowMask, int highMask, int lowFlag, int secondFlag, int highFlag, int fourthFlag)
{
    const int value = suppliedValue >= 0 ? static_cast<int>(suppliedValue) : randomInt(0, 9999);
    const int low = value & 0xff;
    const int high = value >> 8;
    output.push_back(static_cast<char>((low & lowMask) | lowFlag));
    output.push_back(static_cast<char>((low & highMask) | secondFlag));
    output.push_back(static_cast<char>((high & lowMask) | highFlag));
    output.push_back(static_cast<char>((high & highMask) | fourthFlag));
}

std::string bytesToString(const std::vector<uint8_t>& bytes)
{
    return {reinterpret_cast<const char*>(bytes.data()), bytes.size()};
}
}  // namespace

ABogus::ABogus(std::string userAgent, std::string platform)
    : m_userAgentDigest(encodeUserAgent(userAgent))
    , m_browserFingerprint(createBrowserFingerprint(platform))
{
}

std::string ABogus::getValue(const std::string& query, const std::string& body, uint64_t startTime, uint64_t endTime,
                             const std::array<double, 3>& randomValues) const
{
    if (startTime == 0)
    {
        startTime = currentTimeMillis();
    }
    if (endTime == 0)
    {
        endTime = startTime + static_cast<uint64_t>(randomInt(4, 8));
    }

    const auto queryDigest = digestParameter(query);
    const auto bodyDigest = digestParameter(body);
    if (queryDigest.size() < 23 || bodyDigest.size() < 23 || m_userAgentDigest.size() < 25)
    {
        return {};
    }

    const auto corePayload = buildCorePayload(queryDigest, bodyDigest, m_userAgentDigest, startTime, endTime, m_browserFingerprint.size());
    return customBase64(createRandomPrefix(randomValues) + encryptPayload(corePayload, m_browserFingerprint), kABogusAlphabet);
}

std::string ABogus::digestParameter(const std::string& value)
{
    const auto firstDigest = crypto::sm3Raw(value + std::string(kSalt));
    return firstDigest.empty() ? std::string{} : crypto::sm3Raw(firstDigest);
}

std::string ABogus::encodeUserAgent(const std::string& userAgent)
{
    const auto encrypted = crypto::rc4(userAgent, std::string(kUserAgentKey, sizeof(kUserAgentKey) - 1));
    return crypto::sm3Raw(customBase64(encrypted, kUserAgentAlphabet));
}

std::string ABogus::createBrowserFingerprint(const std::string& platform)
{
    if (platform.empty())
    {
        return std::string(kDefaultBrowserFingerprint);
    }

    const int innerWidth = randomInt(1280, 1920);
    const int innerHeight = randomInt(720, 1080);
    const int outerWidth = randomInt(innerWidth, 1920);
    const int outerHeight = randomInt(innerHeight, 1080);
    const int screenY = randomInt(0, 1) == 0 ? 0 : 30;

    std::ostringstream output;
    output << innerWidth << '|' << innerHeight << '|' << outerWidth << '|' << outerHeight << "|0|" << screenY << "|0|0|" << outerWidth << '|' << outerHeight
           << '|' << outerWidth << '|' << outerHeight << '|' << innerWidth << '|' << innerHeight << "|24|24|" << platform;
    return output.str();
}

std::string ABogus::createRandomPrefix(const std::array<double, 3>& randomValues)
{
    std::string prefix;
    prefix.reserve(12);
    appendMaskedRandom(prefix, randomValues[0], 0xaa, 0x2d, 1, 2, 5, 0x28);
    appendMaskedRandom(prefix, randomValues[1], 0xaa, 0x55, 1, 0, 0, 0);
    appendMaskedRandom(prefix, randomValues[2], 0xaa, 0x55, 1, 0, 5, 0);
    return prefix;
}

std::vector<uint8_t> ABogus::buildCorePayload(const std::string& queryDigest, const std::string& bodyDigest, const std::string& userAgentDigest,
                                              uint64_t startTime, uint64_t endTime, std::size_t browserFingerprintSize)
{
    return {44,
            static_cast<uint8_t>((endTime >> 24) & 0xff),
            0,
            0,
            0,
            0,
            24,
            static_cast<uint8_t>(queryDigest[21]),
            static_cast<uint8_t>(bodyDigest[21]),
            0,
            static_cast<uint8_t>(userAgentDigest[23]),
            static_cast<uint8_t>((endTime >> 16) & 0xff),
            0,
            0,
            0,
            1,
            0,
            239,
            static_cast<uint8_t>(queryDigest[22]),
            static_cast<uint8_t>(bodyDigest[22]),
            static_cast<uint8_t>(userAgentDigest[24]),
            static_cast<uint8_t>((endTime >> 8) & 0xff),
            0,
            0,
            0,
            0,
            static_cast<uint8_t>(endTime & 0xff),
            0,
            0,
            14,
            static_cast<uint8_t>((startTime >> 24) & 0xff),
            static_cast<uint8_t>((startTime >> 16) & 0xff),
            0,
            static_cast<uint8_t>((startTime >> 8) & 0xff),
            static_cast<uint8_t>(startTime & 0xff),
            3,
            static_cast<uint8_t>(endTime >> 32),
            1,
            static_cast<uint8_t>(startTime >> 32),
            1,
            static_cast<uint8_t>(browserFingerprintSize),
            0,
            0,
            0};
}

std::string ABogus::encryptPayload(const std::vector<uint8_t>& corePayload, const std::string& browserFingerprint)
{
    std::string payload = bytesToString(corePayload);
    payload += browserFingerprint;
    payload.push_back(static_cast<char>(xorChecksum(corePayload)));
    return crypto::rc4(payload, "y");
}

}  // namespace douyinapi
