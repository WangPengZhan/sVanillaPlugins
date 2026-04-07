#include "XHSUtils.h"

#include <random>
#include <regex>

#include <curl/curl.h>

#include <openssl/md5.h>
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include <Util/LocaleHelper.h>

namespace xhsapi
{
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

    return std::string((char*)digest, digest_len);
}

uint32_t crc32(const std::string& str)
{
    static constexpr uint32_t crc32_table[256] = {
        0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
        0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
        0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
        0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
        0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
        0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
        0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
        0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
        0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
        0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
        0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
        0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
        0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
        0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
        0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
        0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
        0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
        0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
        0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
        0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
        0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
        0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D};

    uint32_t crc = 0xFFFFFFFF;

    for (unsigned char c : str)
    {
        crc = (crc >> 8) ^ crc32_table[(crc ^ c) & 0xFF];
    }

    return crc ^ 0xFFFFFFFF;
}

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

    uint32_t crc = crc32(text);

    std::string a1 = text + std::to_string(crc);
    if (a1.size() > 52)
    {
        a1 = a1.substr(0, 52);
    }

    std::string webId = md5(a1);
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
    std::string decoded_bytes = base64Decode(padded);
    std::vector<uint32_t> codepoints = utf8ToCodePoints(decoded_bytes);

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

}  // namespace xhsapi
