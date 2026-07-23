#include "XHSUtils.h"

#include <algorithm>
#include <random>
#include <regex>
#include <sstream>
#include <array>
#include <chrono>
#include <cctype>
#include <iomanip>
#include <stdexcept>
#include <string_view>

#include <PluginCrypto/Crc32.h>
#include <PluginCrypto/Crypto.h>
#include <PluginCrypto/Encoding.h>
#include <Util/LocaleHelper.h>

namespace xhsapi
{
namespace
{
constexpr std::string_view standardB64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
constexpr std::string_view customB64 = "ZmserbBoHQtNP+wOcza/LpngG8yJq42KWYj0DSfdikx3VT16IlUAFM97hECvuRX5";
constexpr std::string_view x3B64 = "MfgqrsbcyzPQRStuvC7mn501HIJBo2DEFTKdeNOwxWXYZap89+/A4UVLhijkl63G";
constexpr std::string_view xorHex =
    "71a302257793271ddd273bcee3e4b98d9d7935e1da33f5765e2ea8afb6dc77a51a499d23b67c20660025860cbf13d4540d92497f58686c574e508f46e1956344f39139bf4faf22a3eef120b792"
    "58145b2feb5193b6478669961298e79bedca646e1a693a926154a5a7a1bd1cf0dedb742f917a747a1e388b234f2277516db7116035439730fa61e9822a0eca7bff72d8";

std::string encodeCustom(const std::string& input, std::string_view alphabet)
{
    auto output = encoding::base64Encode(input);
    for (auto& character : output)
    {
        const auto index = standardB64.find(character);
        if (index != std::string_view::npos)
            character = alphabet[index];
    }
    return output;
}

std::vector<uint8_t> bytesFromHex(std::string_view input)
{
    std::vector<uint8_t> output;
    for (size_t index = 0; index + 1 < input.size(); index += 2)
        output.push_back(static_cast<uint8_t>(std::stoul(std::string(input.substr(index, 2)), nullptr, 16)));
    return output;
}

void appendLe(std::vector<uint8_t>& output, uint64_t value, size_t length)
{
    for (size_t index = 0; index < length; ++index, value >>= 8)
        output.push_back(static_cast<uint8_t>(value));
}

uint32_t rotateLeft(uint32_t value, unsigned int count)
{
    return (value << count) | (value >> (32 - count));
}

std::array<uint8_t, 16> x3Hash(const std::vector<uint8_t>& input)
{
    uint32_t s0 = 1831565813U, s1 = 461845907U, s2 = 2246822507U, s3 = 3266489909U;
    const auto length = static_cast<uint32_t>(input.size());
    s0 ^= length;
    s1 ^= length << 8;
    s2 ^= length << 16;
    s3 ^= length << 24;
    for (size_t index = 0; index + 7 < input.size(); index += 8)
    {
        const auto read = [&](size_t offset) {
            return static_cast<uint32_t>(input[offset]) | static_cast<uint32_t>(input[offset + 1]) << 8 | static_cast<uint32_t>(input[offset + 2]) << 16 |
                   static_cast<uint32_t>(input[offset + 3]) << 24;
        };
        const auto v0 = read(index), v1 = read(index + 4);
        s0 = rotateLeft((s0 + v0) ^ s2, 7);
        s1 = rotateLeft((v0 ^ s1) + s3, 11);
        s2 = rotateLeft((s2 + v1) ^ s0, 13);
        s3 = rotateLeft((s3 ^ v1) + s1, 17);
    }
    const auto t0 = s0 ^ length, t1 = s1 ^ t0, t2 = s2 + t1, t3 = s3 ^ t2;
    s0 = rotateLeft(t0, 9) + rotateLeft(t2, 17);
    s1 = rotateLeft(t1, 13) ^ rotateLeft(t3, 19);
    s2 = rotateLeft(t2, 17) + s0;
    s3 = rotateLeft(t3, 19) ^ s1;
    std::array<uint8_t, 16> output{};
    const std::array values = {s0, s1, s2, s3};
    for (size_t i = 0; i < values.size(); ++i)
        for (size_t j = 0; j < 4; ++j)
            output[i * 4 + j] = static_cast<uint8_t>(values[i] >> (j * 8));
    return output;
}

int randomInt(int minimum, int maximum)
{
    static thread_local std::mt19937 engine(std::random_device{}());
    return std::uniform_int_distribution<int>(minimum, maximum)(engine);
}

std::string signXyw(const std::string& uri, const std::string& body, const std::string& a1, int64_t timestamp)
{
    const auto message =
        "x1=" + crypto::md5Hex("url=" + uri + body) + ";x2=0|0|0|1|0|0|1|0|0|0|1|0|0|0|0|1|0|0|1;x3=" + a1 + ";x4=" + std::to_string(timestamp) + ";";
    auto payload = crypto::aes128Encrypt(encoding::base64Encode(message), "cbc", "7cc4adla5ay0701v", "4uzjr7mbsibcaldp", "hex");
    std::transform(payload.begin(), payload.end(), payload.begin(), [](unsigned char value) {
        return static_cast<char>(std::tolower(value));
    });
    const nlohmann::json envelope = {
        {"signSvn",     "56"        },
        {"signType",    "x2"        },
        {"appId",       "xhs-pc-web"},
        {"signVersion", "1"         },
        {"payload",     payload     }
    };
    return "XYW_" + encoding::base64Encode(envelope.dump());
}

std::string signXys(const std::string& method, const std::string& uri, const std::string& body, const std::string& a1, int64_t timestamp)
{
    const auto content = uri + body;
    const auto digest = crypto::md5Hex(content);
    const auto pathDigest = method == "GET" ? digest : crypto::md5Hex(uri);
    std::random_device device;
    const auto seed = (static_cast<uint32_t>(device()) << 16) ^ device();
    const auto seedByte = static_cast<uint8_t>(seed);
    std::vector<uint8_t> payload = {121, 104, 96, 41};
    appendLe(payload, seed, 4);
    appendLe(payload, timestamp, 8);
    appendLe(payload, timestamp - randomInt(10, 50) * 1000LL, 8);
    appendLe(payload, randomInt(15, 50), 4);
    appendLe(payload, randomInt(1000, 1200), 4);
    appendLe(payload, content.size(), 4);
    const auto digestBytes = bytesFromHex(digest);
    for (size_t index = 0; index < 8; ++index)
        payload.push_back(digestBytes[index] ^ seedByte);
    payload.push_back(52);
    for (size_t index = 0; index < 52; ++index)
        payload.push_back(index < a1.size() ? static_cast<uint8_t>(a1[index]) : 0);
    constexpr std::string_view appId = "xhs-pc-web";
    payload.push_back(10);
    payload.insert(payload.end(), appId.begin(), appId.end());
    constexpr std::array<uint8_t, 15> env = {115, 248, 83, 102, 103, 201, 181, 131, 99, 94, 4, 68, 250, 132, 21};
    constexpr std::array<uint8_t, 15> checks = {0, 1, 18, 1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0};
    payload.push_back(1);
    payload.push_back(seedByte ^ env[0]);
    for (size_t index = 1; index < env.size(); ++index)
        payload.push_back(env[index] ^ checks[index]);
    payload.insert(payload.end(), {2, 97, 51, 16});
    std::vector<uint8_t> hashInput(payload.begin() + 8, payload.begin() + 16);
    const auto pathBytes = bytesFromHex(pathDigest);
    hashInput.insert(hashInput.end(), pathBytes.begin(), pathBytes.end());
    for (const auto value : x3Hash(hashInput))
        payload.push_back(value ^ seedByte);
    const auto key = bytesFromHex(xorHex);
    for (size_t index = 0; index < payload.size(); ++index)
        payload[index] ^= key[index];
    const std::string raw(reinterpret_cast<const char*>(payload.data()), payload.size());
    const nlohmann::json envelope = {
        {"x0", "4.3.5"},
        {"x1", "xhs-pc-web"},
        {"x2", "Windows"},
        {"x3", "mns0301_" + encodeCustom(raw, x3B64)},
        {"x4", "object"}
    };
    return "XYS_" + encodeCustom(envelope.dump(), customB64);
}

std::string commonSignature(const std::string& a1, int64_t timestamp)
{
    const nlohmann::json fingerprint = {
        {"x33", "0"},
        {"x34", "0"},
        {"x35", "0"},
        {"x36", std::to_string(randomInt(1, 20))},
        {"x37", "0|0|0|0|0|0|0|0|0|1|0|0|0|0|0|0|0|0|1|0|0|0|0|0"},
        {"x38", "0|0|1|0|1|0|0|0|0|0|1|0|1|0|1|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0"},
        {"x39", 0},
        {"x42", "3.4.4"},
        {"x43", "742cc32c"},
        {"x44", std::to_string(timestamp)},
        {"x45", "__SEC_CAV__1-1-1-1-1|__SEC_WSA__|"},
        {"x46", "false"},
        {"x48", ""},
        {"x49", "{list:[],type:}"},
        {"x50", ""},
        {"x51", ""},
        {"x52", ""},
        {"x82", "_0x17a2|_0x1954"}
    };
    const auto quoted = encoding::urlEncode(crypto::rc4(fingerprint.dump(), "xhswebmplfbt"));
    std::string bytes;
    for (size_t percent = quoted.find('%'); percent != std::string::npos && percent + 2 < quoted.size();)
    {
        bytes.push_back(static_cast<char>(std::stoul(quoted.substr(percent + 1, 2), nullptr, 16)));
        const auto next = quoted.find('%', percent + 3);
        bytes.append(quoted, percent + 3, (next == std::string::npos ? quoted.size() : next) - percent - 3);
        percent = next;
    }
    const auto b1 = encodeCustom(bytes, customB64);
    const nlohmann::json common = {
        {"s0",  5                                                      },
        {"s1",  ""                                                     },
        {"x0",  "1"                                                    },
        {"x1",  "4.3.5"                                                },
        {"x2",  "Windows"                                              },
        {"x3",  "xhs-pc-web"                                           },
        {"x4",  "4.86.0"                                               },
        {"x5",  a1                                                     },
        {"x6",  ""                                                     },
        {"x7",  ""                                                     },
        {"x8",  b1                                                     },
        {"x9",  static_cast<int32_t>(checksum::crc32(b1) ^ 0xedb88320U)},
        {"x10", 0                                                      },
        {"x11", "normal"                                               }
    };
    return encodeCustom(common.dump(), customB64);
}
}  // namespace

std::string getCurrentTimeHexMs()
{
    auto now = std::chrono::system_clock::now();
    auto ms = duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    std::stringstream ss;
    ss << std::hex << ms;
    return ss.str();
}

std::string generateRandomString(size_t length, const char* chars)
{
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<> dist(0, strlen(chars) - 1);

    std::string result;
    result.reserve(length);
    for (size_t i = 0; i < length; ++i)
    {
        result += chars[dist(rng)];
    }
    return result;
}

std::pair<std::string, std::string> generateCookieA1AndWebId()
{
    static constexpr char chars[] = "abcdef0123456789";
    static constexpr size_t traceIdLength = 30;
    static constexpr int GET_PLAT_FROM_CODE = 5;

    std::string hexData = getCurrentTimeHexMs();
    std::string randomString = generateRandomString(traceIdLength, chars);
    std::string text = hexData + randomString + "0000";

    uint32_t crc = checksum::crc32(text);

    std::string a1 = text + std::to_string(crc);
    if (a1.size() > 52)
    {
        a1 = a1.substr(0, 52);
    }

    std::string webId = crypto::md5Raw(a1);
    return {a1, webId};
}

std::pair<std::string, std::string> searchJsvmpData(const std::string& data)
{
    std::regex bRegex(R"("b":"(.*?)\",)");
    std::regex dRegex(R"("d":(.*?)\)})");
    std::smatch match;

    std::string b;
    if (std::regex_search(data, match, bRegex) && match.size() > 1)
    {
        b = match[1].str();
    }

    std::string d;
    if (std::regex_search(data, match, dRegex) && match.size() > 1)
    {
        d = match[1].str();
    }
    return {b, d};
}

std::vector<std::vector<int>> decodeJsvmpToLogicList(const std::string& encoded_str)
{
    std::string padded = encoded_str;
    size_t padding = padded.length() % 4;
    if (padding)
    {
        padded.append(4 - padding, '=');
    }
    std::string decoded_bytes = encoding::base64Decode(padded);
    std::vector<uint32_t> codepoints = encoding::utf8ToCodePoints(decoded_bytes);

    std::vector<std::vector<int>> result;
    std::vector<int> current_chunk;
    for (uint32_t cp : codepoints)
    {
        int char_code = static_cast<int>(cp);
        current_chunk.push_back(char_code - 1);
        if (current_chunk.size() == 5)
        {
            result.push_back(std::move(current_chunk));
            current_chunk.clear();
        }
    }
    if (!current_chunk.empty())
    {
        result.push_back(std::move(current_chunk));
    }
    return result;
}

std::string genWebsection(const std::string& data)
{
    auto [b, d] = searchJsvmpData(data);
    std::vector<std::vector<int>> logicList = decodeJsvmpToLogicList(b);

    nlohmann::json dArr = nlohmann::json::parse(d);
    int start = dArr[92].get<int>();
    int end = dArr[93].get<int>();
    if (start < 0 || end >= static_cast<int>(logicList.size()) || start > end)
    {
        return {};
    }

    std::vector<std::vector<int>> targetList = std::vector<std::vector<int>>(logicList.begin() + start, logicList.begin() + end + 1);

    std::vector<int> key(64);
    for (int i = 0; i < 128; i += 2)
    {
        int targetIndex = 675 + i;
        if (targetIndex >= static_cast<int>(targetList.size()))
        {
            return {};
        }
        int dIndex = targetList[targetIndex][2];
        key[i / 2] = dArr[dIndex].get<int>();
    }

    std::string decodeKey;
    decodeKey.reserve(64);
    for (int i = 56; i >= 0; i -= 8)
    {
        for (int j = 0; j < 8; ++j)
        {
            int chCode = key[i + j];
            decodeKey.push_back(static_cast<char>(chCode));
        }
    }
    return decodeKey;
}

SignCookies parseSignCookies(const std::string& cookieHeader)
{
    SignCookies cookies;
    std::istringstream stream(cookieHeader);
    std::string item;
    while (std::getline(stream, item, ';'))
    {
        const auto begin = item.find_first_not_of(" \t");
        const auto equals = item.find('=', begin);
        if (begin != std::string::npos && equals != std::string::npos)
            cookies[item.substr(begin, equals - begin)] = item.substr(equals + 1);
    }
    return cookies;
}

SignatureHeaders signRequest(const std::string& method, const std::string& uri, const std::string& body, const SignCookies& cookies, SignFormat format,
                             std::int64_t timestampMs)
{
    const auto a1 = cookies.find("a1");
    if (a1 == cookies.end() || a1->second.empty())
        throw std::invalid_argument("XHS signing requires the a1 cookie");
    if (timestampMs == 0)
        timestampMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return {format == SignFormat::Xyw ? signXyw(uri, body, a1->second, timestampMs) : signXys(method, uri, body, a1->second, timestampMs),
            commonSignature(a1->second, timestampMs), std::to_string(timestampMs)};
}

}  // namespace xhsapi
