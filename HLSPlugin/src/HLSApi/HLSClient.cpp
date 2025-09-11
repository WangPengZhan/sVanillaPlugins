#include <regex>
#include <filesystem>
#include <sstream>
#include <string>
#include <fstream>
#include <future>

#include "HLSClient.h"
#include "HLSConstants.h"
#include "Util/JsonProcess.h"
#include "Util/LocaleHelper.h"
#include "HLSLog.h"
#include "NetWork/NetworkLog.h"
#include "NetWork/HeaderBodyResponseWrapper.h"
#include "NetWork/CurlCpp/CurlCookie.h"
#include "FFmpeg/FFmpegHelper.h"

#include <openssl/aes.h>
#include <openssl/evp.h>

namespace hlsapi
{

void DecryptFileForTs(const std::string& filename, const KeyInfo& keyInfo)
{
    if (keyInfo.method == KeyInfo::None)
    {
        return;
    }

    if (keyInfo.method == KeyInfo::Aes128)
    {
        std::vector<uint8_t> cipherData;

        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open())
        {
            HLS_LOG_ERROR("Failed to open file: {}", filename);
            return;
        }
        file.seekg(0, std::ios::end);
        cipherData.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(cipherData.data()), cipherData.size());
        file.close();

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx)
        {
            HLS_LOG_ERROR("Failed to create EVP_CIPHER_CTX");
            return;
        }

        std::vector<uint8_t> plainData;
        int len1 = 0, len2 = 0;
        plainData.resize(cipherData.size() + AES_BLOCK_SIZE);

        if (EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, (const unsigned char*)keyInfo.keyFormat.c_str(), (const unsigned char*)keyInfo.iv.c_str()) !=
                1 ||
            EVP_DecryptUpdate(ctx, plainData.data(), &len1, cipherData.data(), (int)cipherData.size()) != 1 ||
            EVP_DecryptFinal_ex(ctx, plainData.data() + len1, &len2) != 1)
        {
            EVP_CIPHER_CTX_free(ctx);
            HLS_LOG_ERROR("EVP_DecryptInit_ex error, Failed to decrypt file: {}", filename);
            return;
        }

        plainData.resize(len1 + len2);
        EVP_CIPHER_CTX_free(ctx);

        std::ofstream outFile(filename, std::ios::binary);
        if (!outFile.is_open())
        {
            HLS_LOG_ERROR("Failed to open output file: {}", filename);
            return;
        }
        outFile.write(reinterpret_cast<const char*>(plainData.data()), plainData.size());
        outFile.close();
    }
    else if (keyInfo.method == KeyInfo::SampleAes)
    {
        // Sample AES decryption is not implemented in this example.
        // You would need to implement the Sample AES decryption logic here.
        HLS_LOG_ERROR("Sample AES decryption is not implemented for file: {}", filename);
    }
    else if (keyInfo.method == KeyInfo::SampleAesCtr)
    {
        // Sample AES CTR decryption is not implemented in this example.
        // You would need to implement the Sample AES CTR decryption logic here.
        HLS_LOG_ERROR("Sample AES CTR decryption is not implemented for file: {}", filename);
    }
    else
    {
        HLS_LOG_ERROR("Unsupported key method: {}", static_cast<int>(keyInfo.method));
    }
}

HLSClient::HLSClient()
{
    initDefaultHeaders();
    initDefaultOptions();
}

HLSClient& HLSClient::globalClient()
{
    static HLSClient hlsClient;
    return hlsClient;
}

std::string HLSClient::downloadM3u8(const std::string& url)
{
    std::string response;
    get(url, response);
    return response;
}

bool HLSClient::downloadTsToFiles(const std::string& uri, const std::string& filename, const KeyInfo& keyInfo)
{
    std::list<std::future<void>> futures;

    std::filesystem::path filePath(util::utf8ToLocale(filename));
    if (std::filesystem::exists(filePath))
    {
        std::filesystem::remove(filePath);
    }

    FILE* file = fopen(filePath.string().c_str(), "wb");
    if (!file)
    {
        std::string str = strerror(errno);
        HLS_LOG_ERROR("Failed to open file: {}, error: {}", filename, str);
        return false;
    }

    get(uri, file);
    fclose(file);

    DecryptFileForTs(filename, keyInfo);

    return true;
}

void HLSClient::mergeTs(const std::string& dir, const std::string& output)
{
    std::map<int, std::string> tsFiles;
    for (const auto& entry : std::filesystem::directory_iterator(dir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".ts")
        {
            auto name = entry.path().stem().string();
            int index = std::stoi(name);
            tsFiles[index] = entry.path().string();
        }
    }

    std::vector<std::string> sortedFiles;
    for (const auto& pair : tsFiles)
    {
        sortedFiles.push_back(pair.second);
    }

    ffmpeg::MergeTsInfo mergeInfo;
    mergeInfo.targetVideo = output;
    ffmpeg::FFmpegHelper().startFFmpeg(
        mergeInfo,
        [mergeInfo]() {
            std::string fileList;
            HLS_LOG_ERROR("Failed to merge TS files: {}, with files: {}", mergeInfo.targetVideo, fileList);
        },
        [mergeInfo, dir]() {
            if (std::filesystem::exists(dir))
            {
                std::filesystem::remove_all(dir);
            }
        });
}

void HLSClient::initDefaultHeaders()
{
    std::string userAgent = std::string("user-agent: ") + network::chrome;
    m_commonHeaders.add(userAgent);
    m_commonHeaders.add(network::accept_language);
    m_commonHeaders.add(network::accept_encoding);
}

void HLSClient::initDefaultOptions()
{
    constexpr time_t timeoutSecond = 5000;
    auto timeout = std::make_shared<network::TimeOut>(timeoutSecond);
    m_commonOptions.insert({timeout->getOption(), timeout});
    auto acceptEncoding = std::make_shared<network::AcceptEncoding>("gzip");
    m_commonOptions.insert({acceptEncoding->getOption(), acceptEncoding});
    auto sslVerifyHost = std::make_shared<network::SSLVerifyHost>(false);
    m_commonOptions.insert({sslVerifyHost->getOption(), sslVerifyHost});
    auto sslVerifyPeer = std::make_shared<network::SSLVerifyPeer>(false);
    m_commonOptions.insert({sslVerifyPeer->getOption(), sslVerifyPeer});
    auto verbose = std::make_shared<network::Verbose>(false);
    m_commonOptions.insert({verbose->getOption(), verbose});
}

}  // namespace hlsapi
