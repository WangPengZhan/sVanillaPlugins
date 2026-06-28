#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <vector>
#include <cstdint>
#include <algorithm>

namespace douyinapi
{

std::vector<uint8_t> stringToBytes(const std::string& s);
std::string bytesToHex(const std::vector<uint8_t>& data);
std::vector<uint8_t> hexToBytes(const std::string& hex);

std::string rc4Encrypt(const std::string& data, const std::string& key);
std::vector<uint8_t> sm3hash(const std::vector<uint8_t>& data);
uint8_t XORChecksum(const std::vector<uint8_t>& a);

class ABogusHash
{
public:
    ABogusHash();

    std::vector<uint8_t> sum(const std::vector<uint8_t>& input);

    static std::vector<uint8_t> hash(const std::vector<uint8_t>& input);

private:
    void compress(const std::vector<uint8_t>& block);
    void write(const std::vector<uint8_t>& input);
    void fill(int length = 60);
    void reset();
    std::vector<uint8_t> reg_to_array();

    static std::vector<uint32_t> generate_f(const std::vector<uint8_t>& e);
    static uint32_t rotl(uint32_t x, int r);
    static uint32_t pe(int e);
    static uint32_t he(int e, uint32_t r, uint32_t t, uint32_t n);
    static uint32_t ve(int e, uint32_t r, uint32_t t, uint32_t n);

private:
    std::vector<uint8_t> m_chunk;
    size_t m_size = 0;
    std::vector<uint32_t> m_reg;

    static std::vector<uint32_t> m_sReg;
};

class ABogus
{
public:
    ABogus(std::string userAgent, std::string platform = "WIN32");

    std::string getValue(const std::string& params, const std::string& method = "GET", uint64_t startTime = 0, uint64_t endTime = 0);

private:
    std::string generateString1(double r1 = -1, double r2 = -1, double r3 = -1);
    std::string generateString2(const std::string& params, const std::string& method = "GET", uint64_t startTime = 0, uint64_t endTime = 0);

    std::vector<uint8_t> generateString2List(const std::string& params, const std::string& method = "GET", uint64_t startTime = 0, uint64_t endTime = 0);

    std::vector<int> list1(double r = -1, int a = 170, int b = 85, int c = 45);
    std::vector<int> list2(double r = -1, int a = 170, int b = 85);
    std::vector<int> list3(double r = -1, int a = 170, int b = 85);
    std::vector<uint8_t> list4(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f, uint8_t g, uint8_t h, uint8_t i, uint8_t j, uint8_t k,
                               uint8_t m, uint8_t n, uint8_t o, uint8_t p, uint8_t q, uint8_t r);

    std::vector<int> randomList(double a = -1, int b = 170, int c = 85, int d = 0, int e = 0, int f = 0, int g = 0);

    std::string fromCharCode(const std::vector<int>& arr);

    std::vector<uint8_t> sm3ToArray(const std::string& data);

    static std::string generateUaCode(const std::string& userAgent);
    static std::string generateBrowserInfo(const std::string& platform);

private:
    std::string m_userAgent;
    std::string m_platform;
    std::string m_uaCode;
    std::string m_browserInfo;
};

}  // namespace douyinapi
