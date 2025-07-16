#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <algorithm>
#include <codecvt>
#include <locale>

#include <curl/curl.h>

#include <openssl/md5.h>
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include "NeteaseCloudMusicUtils.h"

#include "NeteaseCloudMusicApi.h"
#include "NeteaseCloudMusicConstants.h"

namespace
{
std::random_device rd;
std::mt19937 gen(rd());

int getRandomInt(int min, int max)
{
    std::uniform_int_distribution<int> dist(min, max);
    return dist(gen);
}

}  // namespace

namespace netease
{
int generateIPSegment()
{
    return getRandomInt(1, 255);
}

std::string generateRandomChineseIP()
{
    const std::array<std::string, 4> chinaIPPrefixes = {"116.25", "116.76", "116.77", "116.78"};

    // Select a random prefix from the array
    const std::string& randomPrefix = chinaIPPrefixes[getRandomInt(0, chinaIPPrefixes.size() - 1)];

    // Format the IP address using string concatenation
    return randomPrefix + "." + std::to_string(generateIPSegment()) + "." + std::to_string(generateIPSegment());
}

std::string generateRandomString(int length, const std::string& charset)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, charset.size() - 1);

    std::string result;
    for (int i = 0; i < length; ++i)
    {
        result += charset[dis(gen)];
    }
    return result;
}
std::string generateWNMCID()
{
    std::string randomString = generateRandomString(6, "abcdefghijklmnopqrstuvwxyz");
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return randomString + "." + std::to_string(now_ms) + ".01.0";
}

std::string base64Encode(const std::string& input)
{
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO* bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, input.data(), input.length());
    BIO_flush(bio);

    BUF_MEM* bufferPtr;
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string result(bufferPtr->data, bufferPtr->length);

    BIO_free_all(bio);

    return result;
}

std::string base64Decode(const std::string& input)
{
    const auto buffer = new char[input.size()];

    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO* bio = BIO_new_mem_buf(input.c_str(), input.length());
    bio = BIO_push(b64, bio);

    const int length = BIO_read(bio, buffer, input.size());
    std::string result(buffer, length);

    delete[] buffer;
    BIO_free_all(bio);

    return result;
}

std::string hexEncode(const std::string& input)
{
    std::stringstream ss;
    ss << std::hex << std::uppercase;
    for (const unsigned char c : input)
    {
        ss << std::setw(2) << std::setfill('0') << (int)c;
    }
    return ss.str();
}

std::string hexDecode(const std::string& input)
{
    std::string result;
    for (size_t i = 0; i < input.length(); i += 2)
    {
        std::string byteString = input.substr(i, 2);
        const char byte = static_cast<char>(std::stoi(byteString, nullptr, 16));
        result.push_back(byte);
    }
    return result;
}

std::string aesEncrypt(const std::string& text, const std::string& mode, const std::string& key, const std::string& iv, const std::string& format)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    const EVP_CIPHER* cipher = nullptr;

    if (mode == "cbc")
    {
        cipher = EVP_aes_128_cbc();
    }
    else if (mode == "ecb")
    {
        cipher = EVP_aes_128_ecb();
    }
    else
    {
        return "";
    }

    const auto* encryption_key_bytes = reinterpret_cast<const unsigned char*>(key.c_str());
    const auto* iv_data = (unsigned char*)iv.c_str();

    EVP_EncryptInit_ex(ctx, cipher, nullptr, encryption_key_bytes, iv.empty() ? nullptr : iv_data);
    EVP_CIPHER_CTX_set_padding(ctx, 1);

    int max_len = text.length() + EVP_CIPHER_block_size(cipher);
    int c_len = 0;
    int f_len = 0;
    auto* ciphertext = new unsigned char[max_len];

    EVP_EncryptUpdate(ctx, ciphertext, &c_len, (unsigned char*)text.c_str(), text.length());
    EVP_EncryptFinal_ex(ctx, ciphertext + c_len, &f_len);

    const std::string result(reinterpret_cast<char*>(ciphertext), c_len + f_len);
    delete[] ciphertext;
    EVP_CIPHER_CTX_free(ctx);

    if (format == "base64")
    {
        return base64Encode(result);
    }
    else
    {
        return hexEncode(result);
    }
}
std::string aesDecrypt(const std::string& ciphertext, const std::string& key, const std::string& iv, const std::string& format)
{
    std::string decoded;
    if (format == "base64")
    {
        decoded = base64Decode(ciphertext);
    }
    else
    {
        decoded = hexDecode(ciphertext);
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    const EVP_CIPHER* cipher = EVP_aes_128_ecb();

    unsigned char* key_data = (unsigned char*)key.c_str();
    unsigned char* iv_data = (unsigned char*)iv.c_str();

    EVP_DecryptInit_ex(ctx, cipher, NULL, key_data, iv.empty() ? NULL : iv_data);

    int p_len = decoded.length();
    int f_len = 0;
    const auto plaintext = new unsigned char[p_len + AES_BLOCK_SIZE];

    EVP_DecryptUpdate(ctx, plaintext, &p_len, (unsigned char*)decoded.c_str(), decoded.length());
    EVP_DecryptFinal_ex(ctx, plaintext + p_len, &f_len);

    std::string result(reinterpret_cast<char*>(plaintext), p_len + f_len);
    delete[] plaintext;
    EVP_CIPHER_CTX_free(ctx);

    return result;
}

std::string rsaEncrypt(const std::string& str, const std::string& key)
{
    // 创建BIO读取PEM格式的公钥
    BIO* bio = BIO_new_mem_buf(key.c_str(), -1);
    if (!bio)
    {
        return "";
    }

    // 使用EVP_PKEY_new()和PEM_read_bio_PUBKEY()替代RSA相关函数
    EVP_PKEY* pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    if (!pkey)
    {
        BIO_free(bio);
        return "";
    }

    // 创建加密上下文
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, nullptr);
    if (!ctx)
    {
        EVP_PKEY_free(pkey);
        BIO_free(bio);
        return "";
    }

    // 初始化加密操作
    if (EVP_PKEY_encrypt_init(ctx) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        BIO_free(bio);
        return "";
    }

    // 设置RSA加密填充模式为无填充
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_NO_PADDING) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        BIO_free(bio);
        return "";
    }

    RSA* rsa = EVP_PKEY_get1_RSA(pkey);
    int keyLen = RSA_size(rsa);
    RSA_free(rsa);

    std::string padded = str;
    if (padded.size() < keyLen)
    {
        padded.insert(padded.begin(), keyLen - padded.size(), '\0');
    }
    else if (padded.size() > keyLen)
    {
        padded = padded.substr(0, keyLen);
    }

    // 获取输出缓冲区大小
    size_t outlen = 0;
    if (EVP_PKEY_encrypt(ctx, nullptr, &outlen, reinterpret_cast<const unsigned char*>(padded.c_str()), padded.length()) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        BIO_free(bio);
        return "";
    }

    // 分配输出缓冲区
    std::vector<unsigned char> encrypted(outlen);

    // 执行加密操作
    if (EVP_PKEY_encrypt(ctx, encrypted.data(), &outlen, reinterpret_cast<const unsigned char*>(padded.c_str()), padded.length()) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        BIO_free(bio);
        return "";
    }

    // 转换为十六进制字符串
    std::string encryptedStr = hexEncode(std::string(reinterpret_cast<char*>(encrypted.data()), outlen));
    std::transform(encryptedStr.begin(), encryptedStr.end(), encryptedStr.begin(), [](unsigned char c) {
        return std::tolower(c);
    });

    // 清理资源
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    BIO_free(bio);

    return encryptedStr;
}

// std::string rsaEncrypt(const std::string& str, const std::string& key)
//{
//     // 创建BIO读取PEM格式的公钥
//     BIO* bio = BIO_new_mem_buf(key.c_str(), -1);
//     if (!bio)
//     {
//         return "";
//     }
//
//     // 使用EVP_PKEY_new()和PEM_read_bio_PUBKEY()替代RSA相关函数
//     RSA* rsa = PEM_read_bio_RSA_PUBKEY(bio, nullptr, nullptr, nullptr);
//     BIO_free(bio);
//     if (!rsa)
//     {
//         return "";
//     }
//
//     int keyLen = RSA_size(rsa);
//
//     std::string padded = str;
//     if (padded.size() < keyLen)
//     {
//         padded.insert(padded.begin(), keyLen - padded.size(), '\0');
//     }
//     else if (padded.size() > keyLen)
//     {
//         padded = padded.substr(0, keyLen);
//     }
//
//     // 分配输出缓冲区
//     std::vector<unsigned char> encrypted(keyLen, 0);
//     int result = RSA_public_encrypt(keyLen,
//                                     (const unsigned char*)padded.data(),
//                                     encrypted.data(),
//                                     rsa,
//                                     RSA_NO_PADDING
//     );
//     RSA_free(rsa);
//     if (result == -1)
//     {
//         return "";
//     }
//
//     std::string encryptedStr = hexEncode(std::string(reinterpret_cast<char*>(encrypted.data()), result));
//     return encryptedStr;
// }

std::string md5Only(const std::string& input)
{
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx)
    {
        return "";
    }

    if (EVP_DigestInit_ex(ctx, EVP_md5(), nullptr) != 1)
    {
        EVP_MD_CTX_free(ctx);
        return "";
    }

    if (EVP_DigestUpdate(ctx, input.c_str(), input.length()) != 1)
    {
        EVP_MD_CTX_free(ctx);
        return "";
    }

    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len = 0;

    if (EVP_DigestFinal_ex(ctx, digest, &digest_len) != 1)
    {
        EVP_MD_CTX_free(ctx);
        return "";
    }

    EVP_MD_CTX_free(ctx);

    return std::string((char*)digest, digest_len);
}

std::string md5(const std::string& input)
{
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx)
    {
        return "";
    }

    if (EVP_DigestInit_ex(ctx, EVP_md5(), nullptr) != 1)
    {
        EVP_MD_CTX_free(ctx);
        return "";
    }

    if (EVP_DigestUpdate(ctx, input.c_str(), input.length()) != 1)
    {
        EVP_MD_CTX_free(ctx);
        return "";
    }

    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len = 0;

    if (EVP_DigestFinal_ex(ctx, digest, &digest_len) != 1)
    {
        EVP_MD_CTX_free(ctx);
        return "";
    }

    EVP_MD_CTX_free(ctx);

    std::stringstream ss;
    for (unsigned int i = 0; i < digest_len; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    }

    return ss.str();
}

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

std::string cloudMusicDllEncodeID(const std::string& id)
{
    std::string xoredString;
    for (size_t i = 0; i < id.size(); ++i)
    {
        const char ch = id[i] ^ Encrypt::ID_XOR_KEY_1[i % Encrypt::ID_XOR_KEY_1.size()];
        xoredString += ch;
    }

    // 计算MD5摘要
    const std::string digest = md5Only(xoredString);
    // Base64编码
    std::string encoded = base64Encode(digest);
    return encoded;
}

std::unordered_map<std::string, std::string> cookieToJson(const std::string& cookie)
{
    std::unordered_map<std::string, std::string> result;
    if (cookie.empty())
    {
        return result;
    }

    std::string_view str = cookie;
    size_t pos = 0;

    while (pos < str.size())
    {
        // 找到下一个分号
        const size_t next = str.find(';', pos);
        std::string_view item = str.substr(pos, (next == std::string_view::npos ? str.size() : next) - pos);

        // 找到等号
        const size_t eq = item.find('=');
        const auto trim = [](const std::string_view sv) {
            const auto ws = " \t";
            const size_t start = sv.find_first_not_of(ws);
            if (start == std::string_view::npos)
            {
                return std::string_view();
            }
            const size_t end = sv.find_last_not_of(ws);
            return sv.substr(start, end - start + 1);
        };

        if (eq != std::string_view::npos)
        {
            std::string_view key = trim(item.substr(0, eq));
            std::string_view value = trim(item.substr(eq + 1));
            if (!key.empty())
            {  // 跳过空key
                result.emplace(key, value);
            }
        }

        if (next == std::string_view::npos)
        {
            break;
        }
        pos = next + 1;
    }

    return result;
}

std::string cookieObjToString(const std::unordered_map<std::string, std::string>& cookie)
{
    std::stringstream ss;
    bool first = true;

    for (const auto& [fst, snd] : cookie)
    {
        if (!first)
        {
            ss << "; ";
        }
        ss << urlEncode(fst) << "=" << urlEncode(snd);
        first = false;
    }
    return ss.str();
}

std::unordered_map<std::string, std::string> weapi(const nlohmann::ordered_json& object)
{
    const std::string text = object.dump();
    const std::string secretKey = generateRandomString(16, Encrypt::BASE62);

    std::string params = aesEncrypt(aesEncrypt(text, "cbc", Encrypt::PRESET_KEY, Encrypt::IV), "cbc", secretKey, Encrypt::IV);
    std::string reversedKey = secretKey;
    std::ranges::reverse(reversedKey);
    std::string encSecKey = rsaEncrypt(reversedKey, Encrypt::PUBLIC_KEY);

    return {
        {"params",    params   },
        {"encSecKey", encSecKey}
    };
}

std::unordered_map<std::string, std::string> eapi(const std::string& url, const nlohmann::json& object)
{
    const std::string text = object.dump();
    const std::string message = "nobody" + url + "use" + text + "md5forencrypt";
    const std::string digest = md5(message);
    const std::string data = url + "-36cd479b6b5-" + text + "-36cd479b6b5-" + digest;

    return {
        {"params", aesEncrypt(data, "ecb", Encrypt::EAPI_KEY, "", "hex")}
    };
}

nlohmann::json eapiResDecrypt(const std::string& encryptedParams)
{
    std::string decryptedData = aesDecrypt(encryptedParams, Encrypt::EAPI_KEY, "", "hex");
    return nlohmann::json::parse(decryptedData);
}

std::string concatenateCurlHeader(const std::string& name, const std::string& value)
{
    return name + ": " + value + "\r\n";
}

}  // namespace netease
