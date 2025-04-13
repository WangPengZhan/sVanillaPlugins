#include <regex>
#include <filesystem>
#include <sstream>
#include <string>
#include <fstream>

#include "YoutubeClient.h"
#include "YoutubeConstants.h"
#include "Util/JsonProcess.h"
#include "YoutubeLog.h"
#include "YoutubeUtils.h"
#include "NetWork/NetworkLog.h"
#include "NetWork/HeaderBodyResponseWrapper.h"
#include "NetWork/CurlCpp/CurlCookie.h"
#include "NetWork/CurlCpp/CurlOption.h"

namespace youtubeapi
{

std::string extractCipherInfo(const std::string& content)
{
    // std::regex timestampRegex(R"(signatureTimestamp|sts):(\d{5})");
    // std::smatch timestampMatch;
    // if (std::regex_search(content, timestampMatch, timestampRegex))
    //{
    //     std::string signatureTimestamp = timestampMatch[1].str();
    //     if (signatureTimestamp.empty())
    //     {
    //         return std::nullopt;
    //     }

    //    // Find where the player calls the cipher functions
    //    std::regex cipherCallsiteRegex(R"([$_\w]+=function\([$_\w]+\){([$_\w]+)=\1\.split\(['"]{2}\);.*?return \1\.join\(['"]{2}\)})");
    //    std::smatch cipherCallsiteMatch;
    //    if (std::regex_search(content, cipherCallsiteMatch, cipherCallsiteRegex))
    //    {
    //        std::string cipherCallsite = cipherCallsiteMatch[0].str();
    //        if (cipherCallsite.empty())
    //        {
    //            return std::nullopt;
    //        }

    //        // Find the object that defines the cipher functions
    //        std::regex cipherContainerRegex(R"(([$_\w]+)\.[$_\w]+\([$_\w]+,\d+\);)");
    //        std::smatch cipherContainerMatch;
    //        if (std::regex_search(cipherCallsite, cipherContainerMatch, cipherContainerRegex))
    //        {
    //            std::string cipherContainerName = cipherContainerMatch[1].str();
    //            if (cipherContainerName.empty())
    //            {
    //                return std::nullopt;
    //            }

    //            // Find the definition of the cipher functions
    //            std::regex cipherDefinitionRegex(R"(var\s)" + std::regex_escape(cipherContainerName) + R"(={.*?};)");
    //            std::smatch cipherDefinitionMatch;
    //            if (std::regex_search(content, cipherDefinitionMatch, cipherDefinitionRegex))
    //            {
    //                std::string cipherDefinition = cipherDefinitionMatch[0].str();
    //                if (cipherDefinition.empty())
    //                {
    //                    return std::nullopt;
    //                }

    //                // Identify the swap cipher function
    //                std::regex swapFuncRegex(R"(([$_\w]+):function\([$_\w]+,[$_\w]+\){+[^}]*?%[^}]*?})");
    //                std::smatch swapFuncMatch;
    //                std::string swapFuncName;
    //                if (std::regex_search(cipherDefinition, swapFuncMatch, swapFuncRegex))
    //                {
    //                    swapFuncName = swapFuncMatch[1].str();
    //                }

    //                // Identify the splice cipher function
    //                std::regex spliceFuncRegex(R"(([$_\w]+):function\([$_\w]+,[$_\w]+\){+[^}]*?splice[^}]*?})");
    //                std::smatch spliceFuncMatch;
    //                std::string spliceFuncName;
    //                if (std::regex_search(cipherDefinition, spliceFuncMatch, spliceFuncRegex))
    //                {
    //                    spliceFuncName = spliceFuncMatch[1].str();
    //                }

    //                // Identify the reverse cipher function
    //                std::regex reverseFuncRegex(R"(([$_\w]+):function\([$_\w]+\){+[^}]*?reverse[^}]*?})");
    //                std::smatch reverseFuncMatch;
    //                std::string reverseFuncName;
    //                if (std::regex_search(cipherDefinition, reverseFuncMatch, reverseFuncRegex))
    //                {
    //                    reverseFuncName = reverseFuncMatch[1].str();
    //                }

    //                // Extract operations
    //                std::vector<ICipherOperation*> operations;
    //                std::stringstream cipherCallsiteStream(cipherCallsite);
    //                std::string statement;
    //                while (std::getline(cipherCallsiteStream, statement, ';'))
    //                {
    //                    std::regex funcCallRegex(R"([$_\w]+\.([$_\w]+)\([$_\w]+,\d+\))");
    //                    std::smatch funcCallMatch;
    //                    if (std::regex_search(statement, funcCallMatch, funcCallRegex))
    //                    {
    //                        std::string calledFuncName = funcCallMatch[1].str();
    //                        if (calledFuncName.empty())
    //                        {
    //                            continue;
    //                        }

    //                        if (calledFuncName == swapFuncName)
    //                        {
    //                            std::regex indexRegex(R"(\([$_\w]+,(\d+)\))");
    //                            std::smatch indexMatch;
    //                            if (std::regex_search(statement, indexMatch, indexRegex))
    //                            {
    //                                int index = ParseInt(indexMatch[1].str());
    //                                operations.push_back(new SwapCipherOperation(index));
    //                            }
    //                        }
    //                        else if (calledFuncName == spliceFuncName)
    //                        {
    //                            std::regex indexRegex(R"(\([$_\w]+,(\d+)\))");
    //                            std::smatch indexMatch;
    //                            if (std::regex_search(statement, indexMatch, indexRegex))
    //                            {
    //                                int index = ParseInt(indexMatch[1].str());
    //                                operations.push_back(new SpliceCipherOperation(index));
    //                            }
    //                        }
    //                        else if (calledFuncName == reverseFuncName)
    //                        {
    //                            operations.push_back(new ReverseCipherOperation());
    //                        }
    //                    }
    //                }

    //                return CipherManifest(signatureTimestamp, operations);
    //            }
    //        }
    //    }
    //}

    // return std::nullopt;

    return {};
}

YoutubeClient::YoutubeClient()
    : network::NetWork()
    , m_cookies("SOCS=CAISEwgDEgk2NzM5OTg2ODUaAmVuIAEaBgiA6p23Bg; domain=youtube.com")
{
    std::ifstream file("cookie.txt");
    std::stringstream buffer;
    buffer << file.rdbuf();
    m_cookies = buffer.str();
    initDefaultHeaders();
    initDefaultOptions();
}

YoutubeClient& YoutubeClient::globalClient()
{
    static YoutubeClient YoutubeClient;
    return YoutubeClient;
}

bool YoutubeClient::isLogined() const
{
    return !m_cookies.value(youtubeMustKey).empty();
}

void YoutubeClient::setCookies(const std::string& cookies)
{
    m_cookies.setContent(cookies + youtube_default_cookies);
    if (!std::string(m_cookies).empty())
    {
        m_commonOptions[network::CookieFileds::opt] = std::make_shared<network::CookieFileds>(m_cookies);
    }
}

std::string YoutubeClient::getVisitorData()
{
    network::CurlHeader header;
    header.add(youtube_user_agent);
    header.add("Accept: application/json");

    std::string response;
    get(swJsDataUrl, response, header, false, CurlOptions(), false);
    if (response.starts_with(")]}'"))
    {
        response.erase(0, 4);
    }

    std::string result;
    try
    {
        nlohmann::json json = nlohmann::json::parse(response);
        result = json[0][2][0][0][13];
    }
    catch (std::exception& e)
    {
        std::string error = e.what();
    }


    return result;
}

nlohmann::json YoutubeClient::getDataFromRespones(const std::string& respones)
{
    nlohmann::json json;
    try
    {
        std::ofstream file("output.json");
        file << respones;
        file.close();
        json = nlohmann::json::parse(respones);
        util::JsonProcess::removeNullValues(json);
    }
    catch (std::exception& e)
    {
        std::string error = e.what();
    }

    return json;
}

MainResponse YoutubeClient::getVideoInfo(const std::string& videoId)
{
    network::CurlHeader header;
    header.add(youtube_user_agent);
    header.add(youtube_origin);
    // header.add(youtube_accept_encoding);
    // header.add(youtube_accept_language);
    header.add("Content-Type: application/json");

    nlohmann::json content = nlohmann::json::parse(youtubePostContent);
    content["videoId"] = videoId;
    content["context"]["client"]["visitorData"] = visitorData();
    std::string param = content.dump();
    std::string response;
    std::string url = youtubePlayerUrl + std::string("?key=AIzaSyAO_FJ2SlqU8Q4STEHLGCilw_Y9_11qcW8&hl=en&prettyPrint=false");
    post(url, response, param, header, false);

    MainResponse ret;

    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        std::cout << "error: " << e.what() << std::endl;
    }

    return ret;
}

std::string YoutubeClient::getIFrameVersion()
{
    std::string response;
    get(youtubeIFrame, response, network::CurlHeader(), true, CurlOptions(), true);

    std::regex rgx(R"(player\\?/([0-9a-fA-F]{8})\\?/)");
    std::smatch match;

    std::string version;
    if (std::regex_search(response, match, rgx) && match.size() > 1)
    {
        version = match.str(1);
    }

    return version;
}

std::string YoutubeClient::getBaseJs(std::string version)
{
    std::string url = fmt::format(youtubeBaseJs, version);
    std::string response;
    get(url, response, network::CurlHeader(), true, CurlOptions(), true);
    return response;
}

std::vector<AdaptiveFormat> YoutubeClient::getStreamInfo(const std::string& videoId)
{
    auto videoInfo = getVideoInfo(videoId);
    if (videoInfo.streamingData.adaptiveFormats.empty())
    {
        return {};
    }

    std::vector<AdaptiveFormat> video;
    std::vector<AdaptiveFormat> audio;
    for (const auto& adaptiveFormat : videoInfo.streamingData.adaptiveFormats)
    {
        if (adaptiveFormat.mimeType.starts_with("video/mp4"))
        {
            video.push_back(adaptiveFormat);
        }

        if (adaptiveFormat.mimeType.starts_with("audio/mp4"))
        {
            audio.push_back(adaptiveFormat);
        }
    }

    if (video.empty())
    {
        return {};
    }


    std::sort(video.begin(), video.end(), [](const AdaptiveFormat& lhs, const AdaptiveFormat& rhs) {
        if (lhs.width != rhs.width)
        {
            return lhs.width > rhs.width;
        }

        if (lhs.bitrate != rhs.bitrate)
        {
            return lhs.bitrate > rhs.bitrate;
        }

        return lhs.qualityLabel > rhs.qualityLabel;
    });

    std::sort(audio.begin(), audio.end(), [](const AdaptiveFormat& lhs, const AdaptiveFormat& rhs) {
        return lhs.bitrate > rhs.bitrate;
    });

    std::vector result{video.front()};
    if (!audio.empty())
    {
        result.push_back(audio.front());
    }

    return result;
}

bool YoutubeClient::logout()
{
    setCookies("SOCS=CAISEwgDEgk2NzM5OTg2ODUaAmVuIAEaBgiA6p23Bg; domain=youtube.com");
    return true;
}

std::string YoutubeClient::visitorData()
{
    std::lock_guard<std::mutex> lock(m_vistorDataMutex);
    if (!m_visitorData.empty())
    {
        return m_visitorData;
    }

    m_visitorData = getVisitorData();
    return m_visitorData;
}

void YoutubeClient::initDefaultHeaders()
{
    std::string userAgent = std::string("user-agent: ") + network::chrome;
    m_commonHeaders.add(userAgent);
    m_commonHeaders.add(youtube_origin);
}

void YoutubeClient::initDefaultOptions()
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
    if (!std::string(m_cookies).empty())
    {
        m_commonOptions[network::CookieFileds::opt] = std::make_shared<network::CookieFileds>(m_cookies);
    }
}

}  // namespace youtubeapi
