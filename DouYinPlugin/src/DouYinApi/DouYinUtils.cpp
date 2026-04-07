#include "DouYinUtils.h"

#include <sstream>
#include <random>
#include <iostream>

#include <openssl/evp.h>
#include <openssl/rc4.h>
#include <openssl/sha.h>

#include <curl/curl.h>

namespace douyinapi
{
constexpr char endString[] = "cus";
constexpr char uaKey[] = "\u0000\u0001\u000e";
constexpr char browserInfo[] = "1536|742|1536|864|0|0|0|0|1536|864|1536|864|1536|742|24|24|Win32";

static const std::unordered_map<std::string, std::string> strMap = {
    {"s0", "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="},
    {"s1", "Dkdpgh4ZKsQB80/Mfvw36XI1R25+WUAlEi7NLboqYTOPuzmFjJnryx9HVGcaStCe="},
    {"s2", "Dkdpgh4ZKsQB80/Mfvw36XI1R25-WUAlEi7NLboqYTOPuzmFjJnryx9HVGcaStCe="},
    {"s3", "ckdp1h4ZKsUB80/Mfvw36XIgR25+WQAlEi7NLboqYTOPuzmFjJnryx9HVGDaStCe" },
    {"s4", "Dkdpgh2ZmsQB80/MfvV36XI1R45-WUAlEixNLwoqYTOPuzKFjJnry79HbGcaStCe" }
};

std::string urlEncode(const std::string& decoded)
{
    auto* const encodedValue = curl_easy_escape(nullptr, decoded.c_str(), static_cast<int>(decoded.length()));
    std::string result(encodedValue);
    curl_free(encodedValue);
    return result;
}

std::string urlDecode(const std::string& encoded)
{
    int outputLength;
    auto* const decodedValue = curl_easy_unescape(nullptr, encoded.c_str(), static_cast<int>(encoded.length()), &outputLength);
    std::string result(decodedValue, outputLength);
    curl_free(decodedValue);
    return result;
}

std::vector<uint8_t> stringToBytes(const std::string& s)
{
    return std::vector<uint8_t>(s.begin(), s.end());
}

std::string bytesToHex(const std::vector<uint8_t>& data)
{
    std::ostringstream oss;
    for (auto b : data)
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)b;
    }
    return oss.str();
}

std::vector<uint8_t> hexToBytes(const std::string& hex)
{
    std::vector<uint8_t> out;
    for (size_t i = 0; i < hex.size(); i += 2)
    {
        std::string byteString = hex.substr(i, 2);
        out.push_back((uint8_t)strtol(byteString.c_str(), nullptr, 16));
    }
    return out;
}

std::vector<uint32_t> utf8ToCodePoints(const std::string& str)
{
    std::vector<uint32_t> cps;
    const uint8_t* p = (const uint8_t*)str.data();
    size_t len = str.size();
    size_t i = 0;
    while (i < len)
    {
        uint32_t c = p[i];
        if (c < 0x80)
        {
            cps.push_back(c);
            i++;
        }
        else if (c < 0xE0)
        {
            cps.push_back(((c & 0x1F) << 6) | (p[i + 1] & 0x3F));
            i += 2;
        }
        else if (c < 0xF0)
        {
            cps.push_back(((c & 0x0F) << 12) | ((p[i + 1] & 0x3F) << 6) | (p[i + 2] & 0x3F));
            i += 3;
        }
        else
        {
            cps.push_back(((c & 0x07) << 18) | ((p[i + 1] & 0x3F) << 12) | ((p[i + 2] & 0x3F) << 6) | (p[i + 3] & 0x3F));
            i += 4;
        }
    }
    return cps;
}

std::string codePointsToUtf8(const std::vector<uint32_t>& cps)
{
    std::string out;
    for (uint32_t c : cps)
    {
        if (c < 0x80)
        {
            out += (char)c;
        }
        else if (c < 0x800)
        {
            out += (char)(0xC0 | (c >> 6));
            out += (char)(0x80 | (c & 0x3F));
        }
        else if (c < 0x10000)
        {
            out += (char)(0xE0 | (c >> 12));
            out += (char)(0x80 | ((c >> 6) & 0x3F));
            out += (char)(0x80 | (c & 0x3F));
        }
        else
        {
            out += (char)(0xF0 | (c >> 18));
            out += (char)(0x80 | ((c >> 12) & 0x3F));
            out += (char)(0x80 | ((c >> 6) & 0x3F));
            out += (char)(0x80 | (c & 0x3F));
        }
    }
    return out;
}

std::string rc4Encrypt(const std::string& data, const std::string& key)
{
    std::vector<uint32_t> dataCP = utf8ToCodePoints(data);
    std::vector<uint32_t> keyCP = utf8ToCodePoints(key);

    uint8_t s[256];
    for (int i = 0; i < 256; ++i)
    {
        s[i] = (uint8_t)i;
    }

    int j = 0;
    size_t keyLen = keyCP.size();
    for (int i = 0; i < 256; ++i)
    {
        j = (j + s[i] + (keyCP[i % keyLen] & 0xFF)) % 256;
        std::swap(s[i], s[j]);
    }

    int i = 0;
    j = 0;
    std::vector<uint32_t> outCP;
    for (uint32_t cp : dataCP)
    {
        i = (i + 1) % 256;
        j = (j + s[i]) % 256;
        std::swap(s[i], s[j]);
        uint8_t t = (s[i] + s[j]) % 256;
        outCP.push_back(cp ^ s[t]);
    }

    return codePointsToUtf8(outCP);
}

std::vector<uint8_t> sm3hash(const std::vector<uint8_t>& data)
{
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    const EVP_MD* md = EVP_sm3();

    std::vector<uint8_t> out(EVP_MAX_MD_SIZE);
    unsigned int len = 0;

    EVP_DigestInit_ex(ctx, md, nullptr);
    EVP_DigestUpdate(ctx, data.data(), data.size());
    EVP_DigestFinal_ex(ctx, out.data(), &len);

    EVP_MD_CTX_free(ctx);

    out.resize(len);
    return out;
}

uint8_t XORChecksum(const std::vector<uint8_t>& a)
{
    uint8_t r = 0;
    for (uint8_t i : a)
    {
        r ^= i;
    }
    return r;
}

std::string generateResult(const std::string& s, const std::string& e = "s4")
{
    const std::string& table = strMap.at(e);
    std::string r;
    r.reserve((s.size() + 2) / 3 * 4);

    std::vector<uint32_t> chars = utf8ToCodePoints(s);
    size_t len = chars.size();

    for (size_t i = 0; i < len; i += 3)
    {
        uint32_t n = 0;

        if (i + 2 < len)
        {
            n = (chars[i] << 16) | (chars[i + 1] << 8) | chars[i + 2];
        }
        else if (i + 1 < len)
        {
            n = (chars[i] << 16) | (chars[i + 1] << 8);
        }
        else
        {
            n = chars[i] << 16;
        }

        const int shifts[] = {18, 12, 6, 0};
        const uint32_t masks[] = {0xFC0000, 0x03F000, 0x00FC0, 0x0003F};

        for (int idx = 0; idx < 4; ++idx)
        {
            int j = shifts[idx];
            uint32_t k = masks[idx];

            if (j == 6 && i + 1 >= len)
            {
                break;
            }
            if (j == 0 && i + 2 >= len)
            {
                break;
            }

            uint32_t v = (n & k) >> j;
            r.push_back(table[v]);
        }
    }

    size_t pad = (4 - r.size() % 4) % 4;
    r.append(pad, '=');

    return r;
}

std::vector<uint32_t> ABogusHash::m_sReg = {1937774191u, 1226093241u, 388252375u, 3666478592u, 2842636476u, 372324522u, 3817729613u, 2969243214u};

ABogusHash::ABogusHash()
{
    reset();
}

std::vector<uint8_t> ABogusHash::sum(const std::vector<uint8_t>& input)
{
    reset();
    write(input);
    fill();
    compress(m_chunk);
    return reg_to_array();
}

std::vector<uint8_t> ABogusHash::hash(const std::vector<uint8_t>& input)
{
    ABogusHash hash;
    return hash.sum(input);
}

void ABogusHash::compress(const std::vector<uint8_t>& block)
{
    auto f = generate_f(block);
    std::vector<uint32_t> i = m_reg;

    for (int o = 0; o < 64; ++o)
    {
        uint32_t c = rotl(i[0], 12) + i[4] + rotl(pe(o), o);
        c = rotl(c, 7);

        uint32_t s = c ^ rotl(i[0], 12);

        uint32_t u = he(o, i[0], i[1], i[2]);
        u = u + i[3] + s + f[o + 68];

        uint32_t b = ve(o, i[4], i[5], i[6]);
        b = b + i[7] + c + f[o];

        i[3] = i[2];
        i[2] = rotl(i[1], 9);
        i[1] = i[0];
        i[0] = u;

        i[7] = i[6];
        i[6] = rotl(i[5], 19);
        i[5] = i[4];
        i[4] = b ^ rotl(b, 9) ^ rotl(b, 17);
    }

    for (int l = 0; l < 8; ++l)
    {
        m_reg[l] ^= i[l];
    }
}

void ABogusHash::write(const std::vector<uint8_t>& input)
{
    m_size = input.size();

    if (input.size() <= 64)
    {
        m_chunk = input;
    }
    else
    {
        for (size_t i = 0; i + 64 < input.size(); i += 64)
        {
            std::vector<uint8_t> block(input.begin() + i, input.begin() + i + 64);
            compress(block);
        }

        m_chunk.assign(input.end() - (input.size() % 64), input.end());
    }
}

void ABogusHash::reset()
{
    m_chunk.clear();
    m_size = 0;
    m_reg = m_sReg;
}

void ABogusHash::fill(int length)
{
    uint32_t bit_size = static_cast<uint32_t>(m_size * 8);

    m_chunk.push_back(128);

    while (m_chunk.size() < static_cast<size_t>(length))
        m_chunk.push_back(0);

    for (int i = 0; i < 4; ++i)
    {
        m_chunk.push_back((bit_size >> (8 * (3 - i))) & 0xFF);
    }
}

std::vector<uint8_t> ABogusHash::reg_to_array()
{
    std::vector<uint8_t> out(32);

    for (int i = 0; i < 8; ++i)
    {
        uint32_t c = m_reg[i];

        out[4 * i + 3] = c & 0xFF;
        c >>= 8;
        out[4 * i + 2] = c & 0xFF;
        c >>= 8;
        out[4 * i + 1] = c & 0xFF;
        c >>= 8;
        out[4 * i + 0] = c & 0xFF;
    }

    return out;
}
std::vector<uint32_t> ABogusHash::generate_f(const std::vector<uint8_t>& e)
{
    std::vector<uint32_t> r(132, 0);

    for (int t = 0; t < 16; ++t)
    {
        r[t] = (uint32_t(e[4 * t]) << 24) | (uint32_t(e[4 * t + 1]) << 16) | (uint32_t(e[4 * t + 2]) << 8) | (uint32_t(e[4 * t + 3]));
    }

    for (int n = 16; n < 68; ++n)
    {
        uint32_t a = r[n - 16] ^ r[n - 9] ^ rotl(r[n - 3], 15);
        a = a ^ rotl(a, 15) ^ rotl(a, 23);
        r[n] = a ^ rotl(r[n - 13], 7) ^ r[n - 6];
    }

    for (int n = 68; n < 132; ++n)
    {
        r[n] = r[n - 68] ^ r[n - 64];
    }

    return r;
}

uint32_t ABogusHash::rotl(uint32_t x, int r)
{
    r %= 32;
    return (x << r) | (x >> (32 - r));
}

uint32_t ABogusHash::pe(int e)
{
    return (e < 16) ? 2043430169u : 2055708042u;
}

uint32_t ABogusHash::he(int e, uint32_t r, uint32_t t, uint32_t n)
{
    if (e < 16)
        return (r ^ t ^ n);
    return (r & t) | (r & n) | (t & n);
}

uint32_t ABogusHash::ve(int e, uint32_t r, uint32_t t, uint32_t n)
{
    if (e < 16)
        return (r ^ t ^ n);
    return (r & t) | (~r & n);
}

ABogus::ABogus(std::string userAgent, std::string platform)
    : m_userAgent(std::move(userAgent))
    , m_platform(std::move(platform))
{
    m_uaCode = generateUaCode(m_userAgent);
    m_browserInfo = generateBrowserInfo(m_platform);
}

std::string ABogus::getValue(const std::string& params, const std::string& method, uint64_t startTime, uint64_t endTime)
{
    std::string str1 = generateString1(-1, -1, -1);
    std::string str2 = generateString2(params, method, startTime, endTime);
    std::string result = generateResult(str1 + str2, "s4");
    return result;
}

std::string ABogus::generateString1(double r1, double r2, double r3)
{
    std::string s;
    s += fromCharCode(list1(r1));
    s += fromCharCode(list2(r2));
    s += fromCharCode(list3(r3));
    return s;
}

std::string ABogus::generateString2(const std::string& urlParams, const std::string& method, uint64_t startTime, uint64_t endTime)
{
    std::vector<uint8_t> result = generateString2List(urlParams, method, startTime, endTime);
    uint8_t xos = XORChecksum(result);
    result.insert(result.end(), m_browserInfo.begin(), m_browserInfo.end());
    result.push_back(xos);

    std::string data(result.begin(), result.end());
    return rc4Encrypt(data, "y");
}

std::vector<uint8_t> ABogus::generateString2List(const std::string& urlParams, const std::string& method, uint64_t startTime, uint64_t endTime)
{
    if (startTime == 0)
    {
        startTime = static_cast<uint64_t>(std::time(nullptr) * 1000ULL);
    }

    if (endTime == 0)
    {
        endTime = startTime + (rand() % 5 + 4);
    }

    auto paramsArray = sm3ToArray(urlParams);
    auto methodArray = sm3ToArray(method);

    std::vector<uint8_t> result =
        list4((endTime >> 24) & 0xFF, paramsArray[21], uint8_t(m_uaCode[23]), (endTime >> 16) & 0xFF, paramsArray[22], uint8_t(m_uaCode[24]),
              (endTime >> 8) & 0xFF, (endTime >> 0) & 0xFF, (startTime >> 24) & 0xFF, (startTime >> 16) & 0xFF, (startTime >> 8) & 0xFF,
              (startTime >> 0) & 0xFF, methodArray[21], methodArray[22], static_cast<uint32_t>(endTime / 256 / 256 / 256 / 256),
              static_cast<uint32_t>(startTime / 256 / 256 / 256 / 256), uint8_t(m_browserInfo.size()));

    return result;
}

std::vector<int> ABogus::list1(double r, int a, int b, int c)
{
    return randomList(r, a, c, 1, 2, 5, c & a);
}

std::vector<int> ABogus::list2(double r, int a, int b)
{
    return randomList(r, a, b, 1, 0, 0, 0);
}

std::vector<int> ABogus::list3(double r, int a, int b)
{
    return randomList(r, a, b, 1, 0, 5, 0);
}

std::vector<uint8_t> ABogus::list4(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f, uint8_t g, uint8_t h, uint8_t i, uint8_t j, uint8_t k,
                                   uint8_t m, uint8_t n, uint8_t o, uint8_t p, uint8_t q, uint8_t r)
{
    return {44, a, 0, 0, 0, 0, 24, b, n, 0, c, d, 0, 0, 0, 1, 0, 239, e, o, f, g, 0, 0, 0, 0, h, 0, 0, 14, i, j, 0, k, m, 3, p, 1, q, 1, r, 0, 0, 0};
}

std::vector<int> ABogus::randomList(double a, int b, int c, int d, int e, int f, int g)
{
    double r = (a != -1) ? a : (rand() % 10000);
    int ri = (int)r;

    std::vector<int> v = {ri, ri & 255, ri >> 8};

    v.push_back((v[1] & b) | d);
    v.push_back((v[1] & c) | e);
    v.push_back((v[2] & b) | f);
    v.push_back((v[2] & c) | g);

    return std::vector<int>(v.end() - 4, v.end());
}

std::string ABogus::fromCharCode(const std::vector<int>& arr)
{
    std::string s;
    for (auto v : arr)
        s.push_back((char)v);
    return s;
}

std::vector<uint8_t> ABogus::sm3ToArray(const std::string& data)
{
    std::string allData = data + endString;
    return sm3hash(sm3hash(std::vector<uint8_t>(allData.begin(), allData.end())));
}

std::string ABogus::generateUaCode(const std::string& userAgent)
{
    std::string result = rc4Encrypt(userAgent, std::string(uaKey, sizeof(uaKey) - 1));
    std::cout << "result = " << bytesToHex(stringToBytes(result)) << std::endl;
    result = generateResult(result, "s3");
    std::vector<uint8_t> hash = ABogusHash::hash(std::vector<uint8_t>(result.begin(), result.end()));
    return std::string(hash.begin(), hash.end());
}

std::string ABogus::generateBrowserInfo(const std::string& platform)
{
    if (platform.empty())
    {
        return browserInfo;
    }

    static thread_local std::mt19937 rng{std::random_device{}()};

    std::uniform_int_distribution<int> w_dist(1280, 1920);
    std::uniform_int_distribution<int> h_dist(720, 1080);
    std::uniform_int_distribution<int> y_dist(0, 1);

    int inner_width = w_dist(rng);
    int inner_height = h_dist(rng);

    std::uniform_int_distribution<int> ow_dist(inner_width, 1920);
    std::uniform_int_distribution<int> oh_dist(inner_height, 1080);

    int outer_width = ow_dist(rng);
    int outer_height = oh_dist(rng);

    int screen_x = 0;
    int screen_y = (y_dist(rng) == 0) ? 0 : 30;

    std::ostringstream oss;
    oss << inner_width << '|' << inner_height << '|' << outer_width << '|' << outer_height << '|' << screen_x << '|' << screen_y << '|' << 0 << '|' << 0 << '|'
        << outer_width << '|' << outer_height << '|' << outer_width << '|' << outer_height << '|' << inner_width << '|' << inner_height << '|' << 24 << '|'
        << 24 << '|' << platform;

    return oss.str();
}

}  // namespace douyinapi
