#include "HLSClient.h"

#include <array>
#include <charconv>
#include <filesystem>
#include <fstream>
#include <limits>
#include <map>
#include <utility>

#include <openssl/evp.h>

#include "FFmpeg/FFmpegHelper.h"
#include "HLSLog.h"
#include "Util/LocaleHelper.h"

namespace hlsapi
{
namespace
{
constexpr long kTimeoutSecond = 30;

bool decodeIv(std::string_view text, std::uint64_t sequence, std::array<unsigned char, 16>& iv)
{
    iv.fill(0);
    if (text.empty())
    {
        for (std::size_t index = 0; index < sizeof(sequence); ++index)
        {
            iv[15 - index] = static_cast<unsigned char>(sequence >> (index * 8));
        }
        return true;
    }

    if (text.starts_with("0x") || text.starts_with("0X"))
    {
        text.remove_prefix(2);
    }
    if (text.empty() || text.size() > 32 || text.size() % 2 != 0)
    {
        return false;
    }

    const std::size_t firstByte = 16 - text.size() / 2;
    for (std::size_t index = 0; index < text.size(); index += 2)
    {
        unsigned int byte = 0;
        const auto [end, error] = std::from_chars(text.data() + index, text.data() + index + 2, byte, 16);
        if (error != std::errc{} || end != text.data() + index + 2)
        {
            return false;
        }
        iv[firstByte + index / 2] = static_cast<unsigned char>(byte);
    }
    return true;
}

bool readFile(const std::filesystem::path& path, std::vector<std::uint8_t>& data)
{
    std::error_code error;
    const auto size = std::filesystem::file_size(path, error);
    if (error)
    {
        HLS_LOG_ERROR("Unable to get downloaded HLS file size, file: {}, error: {}", util::localeToUtf8(path.string()), error.message());
        return false;
    }
    if (size == 0)
    {
        HLS_LOG_ERROR("Downloaded HLS file is empty, file: {}", util::localeToUtf8(path.string()));
        return false;
    }
    std::ifstream input(path, std::ios::binary);
    if (!input.is_open())
    {
        HLS_LOG_ERROR("Unable to open downloaded HLS file, file: {}", util::localeToUtf8(path.string()));
        return false;
    }
    data.resize(static_cast<std::size_t>(size));
    input.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(size));
    if (input.gcount() != static_cast<std::streamsize>(size))
    {
        HLS_LOG_ERROR("Unable to read complete downloaded HLS file, file: {}, expected: {}, actual: {}", util::localeToUtf8(path.string()), size,
                      input.gcount());
        return false;
    }
    return true;
}

bool writeFile(const std::filesystem::path& path, const std::vector<std::uint8_t>& data)
{
    if (data.empty())
    {
        HLS_LOG_ERROR("Unable to write empty HLS output file, file: {}", util::localeToUtf8(path.string()));
        return false;
    }
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output.is_open())
    {
        HLS_LOG_ERROR("Unable to open HLS output file, file: {}", util::localeToUtf8(path.string()));
        return false;
    }
    output.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    output.close();
    if (!output.good())
    {
        HLS_LOG_ERROR("Unable to write HLS output file, file: {}", util::localeToUtf8(path.string()));
        return false;
    }
    return true;
}
}  // namespace

bool decryptSegment(const std::vector<std::uint8_t>& cipherData, const KeyInfo& keyInfo, std::uint64_t mediaSequence, std::vector<std::uint8_t>& plainData)
{
    plainData.clear();
    if (keyInfo.method == KeyInfo::None)
    {
        plainData = cipherData;
        return true;
    }
    if (keyInfo.method != KeyInfo::Aes128)
    {
        HLS_LOG_ERROR("Unsupported HLS encryption method: {}", keyInfo.strMethod);
        return false;
    }
    if (!keyInfo.keyFormat.empty() && keyInfo.keyFormat != "identity")
    {
        HLS_LOG_ERROR("Unsupported HLS key format: {}", keyInfo.keyFormat);
        return false;
    }
    if (keyInfo.key.size() != 16)
    {
        HLS_LOG_ERROR("Invalid AES-128 HLS key length: {}", keyInfo.key.size());
        return false;
    }
    if (cipherData.empty() || cipherData.size() % 16 != 0)
    {
        HLS_LOG_ERROR("Invalid AES-128 HLS ciphertext length: {}", cipherData.size());
        return false;
    }
    if (cipherData.size() > static_cast<std::size_t>((std::numeric_limits<int>::max)() - EVP_MAX_BLOCK_LENGTH))
    {
        HLS_LOG_ERROR("AES-128 HLS ciphertext is too large: {}", cipherData.size());
        return false;
    }

    std::array<unsigned char, 16> iv{};
    if (!decodeIv(keyInfo.iv, mediaSequence, iv))
    {
        HLS_LOG_ERROR("Invalid AES-128 HLS IV: {}", keyInfo.iv);
        return false;
    }

    EVP_CIPHER_CTX* context = EVP_CIPHER_CTX_new();
    if (!context)
    {
        HLS_LOG_ERROR("Unable to create AES-128 HLS decryption context");
        return false;
    }
    plainData.resize(cipherData.size() + EVP_MAX_BLOCK_LENGTH);
    int firstLength = 0;
    int finalLength = 0;
    if (EVP_DecryptInit_ex(context, EVP_aes_128_cbc(), nullptr, reinterpret_cast<const unsigned char*>(keyInfo.key.data()), iv.data()) != 1)
    {
        HLS_LOG_ERROR("Unable to initialize AES-128 HLS decryption");
        EVP_CIPHER_CTX_free(context);
        plainData.clear();
        return false;
    }
    if (EVP_DecryptUpdate(context, plainData.data(), &firstLength, cipherData.data(), static_cast<int>(cipherData.size())) != 1)
    {
        HLS_LOG_ERROR("Unable to decrypt AES-128 HLS data");
        EVP_CIPHER_CTX_free(context);
        plainData.clear();
        return false;
    }
    if (EVP_DecryptFinal_ex(context, plainData.data() + firstLength, &finalLength) != 1)
    {
        HLS_LOG_ERROR("Unable to finalize AES-128 HLS decryption; key, IV, or ciphertext may be invalid");
        EVP_CIPHER_CTX_free(context);
        plainData.clear();
        return false;
    }
    EVP_CIPHER_CTX_free(context);
    plainData.resize(static_cast<std::size_t>(firstLength + finalLength));
    return true;
}

bool mergeTsFiles(const std::string& dir, const std::vector<std::string>& files, const std::string& output, std::function<void()> onSuccess)
{
    if (dir.empty() || files.empty() || output.empty())
    {
        HLS_LOG_ERROR("Cannot merge HLS segments without a directory, inputs, and output");
        return false;
    }
    const auto concatPath = std::filesystem::path(util::utf8ToLocale(dir)) / "concat.txt";
    std::ofstream concat(concatPath, std::ios::trunc);
    if (!concat.is_open())
    {
        HLS_LOG_ERROR("HLS merge failed, cannot write concat list: {}", util::localeToUtf8(concatPath.string()));
        return false;
    }
    for (const auto& file : files)
    {
        concat << "file '" << std::filesystem::path(util::utf8ToLocale(file)).filename().string() << "'\n";
    }
    concat.close();
    if (!concat.good())
    {
        HLS_LOG_ERROR("Unable to finish writing HLS concat list");
        return false;
    }

    ffmpeg::MergeTsInfo mergeInfo;
    mergeInfo.concatFile = util::localeToUtf8(concatPath.string());
    mergeInfo.targetVideo = util::localeToUtf8(output);
    return ffmpeg::FFmpegHelper().startFFmpeg(mergeInfo, [] {}, std::move(onSuccess));
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

std::string HLSClient::downloadM3u8(const std::string& url, std::string* effectiveUrl)
{
    StringAndEffectiveUrl response;
    if (!get(url, response))
    {
        HLS_LOG_ERROR("HLS playlist request failed, url: {}", url);
        return {};
    }
    if (response.body.empty())
    {
        HLS_LOG_ERROR("HLS playlist request failed, url: {}", url);
        return {};
    }
    if (effectiveUrl)
    {
        *effectiveUrl = response.effectiveUrl;
    }
    return response.body;
}

bool HLSClient::downloadKey(const std::string& url, std::string& key)
{
    std::string response;
    if (!get(url, response))
    {
        HLS_LOG_ERROR("HLS key request failed, url: {}", url);
        key.clear();
        return false;
    }
    if (response.empty())
    {
        HLS_LOG_ERROR("HLS key request failed, url: {}", url);
        key.clear();
        return false;
    }
    key = std::move(response);
    return true;
}

bool HLSClient::downloadTsToFiles(const std::string& uri, const std::string& filename, const KeyInfo& keyInfo, std::uint64_t mediaSequence,
                                  std::size_t byteRangeLength, std::size_t byteRangeOffset)
{
    if (filename.empty())
    {
        HLS_LOG_ERROR("HLS segment download failed, empty output filename, uri: {}", uri);
        return false;
    }

    const std::filesystem::path outputPath(util::utf8ToLocale(filename));
    const auto transferPath = outputPath.string() + ".part";
    std::error_code error;
    std::filesystem::remove(outputPath, error);
    std::filesystem::remove(transferPath, error);

    std::vector<std::uint8_t> cipherData;
    std::vector<std::uint8_t> plainData;
    network::CurlHeader headers;
    if (byteRangeLength != 0)
    {
        if (byteRangeLength - 1 > (std::numeric_limits<std::size_t>::max)() - byteRangeOffset)
        {
            HLS_LOG_ERROR("HLS byte range overflows, uri: {}, offset: {}, length: {}", uri, byteRangeOffset, byteRangeLength);
            return false;
        }
        headers.add("Range: bytes=" + std::to_string(byteRangeOffset) + "-" + std::to_string(byteRangeOffset + byteRangeLength - 1));
    }
    if (!network::downloadFileChecked(*this, uri, std::filesystem::path(transferPath), headers, true))
    {
        HLS_LOG_ERROR("HLS segment HTTP download failed, uri: {}", uri);
        std::filesystem::remove(transferPath, error);
        return false;
    }
    if (!readFile(transferPath, cipherData))
    {
        HLS_LOG_ERROR("Unable to load downloaded HLS segment, uri: {}", uri);
        std::filesystem::remove(transferPath, error);
        return false;
    }
    if (byteRangeLength != 0 && cipherData.size() != byteRangeLength)
    {
        HLS_LOG_ERROR("HLS byte range response size mismatch, uri: {}, expected: {}, actual: {}", uri, byteRangeLength, cipherData.size());
        std::filesystem::remove(transferPath, error);
        return false;
    }
    if (!decryptSegment(cipherData, keyInfo, mediaSequence, plainData))
    {
        HLS_LOG_ERROR("Unable to decrypt HLS segment, uri: {}", uri);
        std::filesystem::remove(transferPath, error);
        return false;
    }
    if (!writeFile(outputPath, plainData))
    {
        HLS_LOG_ERROR("Unable to save HLS segment, file: {}", filename);
        std::filesystem::remove(transferPath, error);
        return false;
    }
    std::filesystem::remove(transferPath, error);
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
    constexpr long timeoutSecond = 5;
    auto timeout = std::make_shared<network::TimeOut>(timeoutSecond);
    m_commonOptions.insert({timeout->getOption(), timeout});
    auto acceptEncoding = std::make_shared<network::AcceptEncoding>("gzip");
    m_commonOptions.insert({acceptEncoding->getOption(), acceptEncoding});
}

}  // namespace hlsapi
