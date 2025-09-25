#include <regex>
#include <filesystem>
#include <sstream>
#include <string>
#include <fstream>

#include <libxml/HTMLparser.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "YoutubeClient.h"
#include "YoutubeConstants.h"
#include "Util/JsonProcess.h"
#include "YoutubeLog.h"
#include "YoutubeUtils.h"
#include "NetWork/HeaderBodyResponseWrapper.h"
#include "NetWork/CurlCpp/CurlCookie.h"
#include "NetWork/CurlCpp/CurlOption.h"

struct LocationUrl
{
    std::string locationUrl;
    std::string body;
};
namespace network
{

template <typename T>
class CurlResponseWrapper;

template <>
class CurlResponseWrapper<LocationUrl>
{
public:
    CurlResponseWrapper(LocationUrl& response)
        : m_response(response)
    {
    }

    void setToCurl(CURL* handle)
    {
        if (handle)
        {
            curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeFunc<std::string>);
            curl_easy_setopt(handle, CURLOPT_WRITEDATA, &m_response.body);
        }
    }

    void setToCurl(CurlEasy& easy)
    {
        setToCurl(easy.handle());
    }

    void readAfter(CURL* handle)
    {
        char* redirectUrl = nullptr;
        curl_easy_getinfo(handle, CURLINFO_REDIRECT_URL, &redirectUrl);
        if (redirectUrl)
        {
            m_response.locationUrl = redirectUrl;
        }
    }

    void readAfter(CurlEasy& easy)
    {
        readAfter(easy.handle());
    }

private:
    LocationUrl& m_response;
};
}  // namespace network

namespace youtubeapi
{

YoutubeClient::YoutubeClient()
    : network::NetWork()
    , m_cookies("SOCS=CAISEwgDEgk2NzM5OTg2ODUaAmVuIAEaBgiA6p23Bg; domain=.youtube.com")
{
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
    return !m_cookies.cookie(domain).value(youtubeMustKey).empty();
}

void YoutubeClient::setCookie(const std::string& cookie)
{
    network::CurlCookies cookies;
    cookies.addCurlCookie(network::CurlCookie::parseCookie(cookie + youtube_default_cookies));
    m_cookies = cookies;

    if (!std::string(m_cookies).empty())
    {
        std::lock_guard lock(m_mutexRequest);
        m_commonOptions[network::CookieFields::opt] = std::make_shared<network::CookieFields>(m_cookies.cookie(domain));
    }
}

void YoutubeClient::setCookies(const std::string& cookies)
{
    std::lock_guard lk(m_mutexRequest);
    m_cookies = network::CurlCookies(cookies);
    m_commonOptions[network::CookieFields::opt] = std::make_shared<network::CookieFields>(m_cookies.cookie(domain));
}

std::string YoutubeClient::cookies() const
{
    return std::string(m_cookies);
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
        YOUTUBE_LOG_WARN("getVisitorData error: {}", error);
    }

    return result;
}

nlohmann::json YoutubeClient::getDataFromRespones(const std::string& respones)
{
    nlohmann::json json;
    try
    {
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
    YOUTUBE_LOG_WARN("getVideoInfo videoId: {}", videoId);
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
        YOUTUBE_LOG_WARN("getVideoInfo error: {}", e.what());
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
    YOUTUBE_LOG_INFO("getBaseJs version: {}", version);
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
        YOUTUBE_LOG_WARN("getStreamInfo adaptiveFormats is empty videoId: {}", videoId);
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
        YOUTUBE_LOG_WARN("getStreamInfo video is empty videoId: {}", videoId);
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

PlayListInfo YoutubeClient::playlistInfo(const std::string& listId)
{
    YOUTUBE_LOG_WARN("playlistInfo videoId: {}", listId);
    network::CurlHeader header;
    header.add("Content-Type: application/json");
    nlohmann::json content = nlohmann::json::parse(youtubeBrowsePostContent);
    content["browseId"] = "VL" + listId;
    std::string param = content.dump();
    std::string response;
    post(youtubeBrowseUrl, response, param, header, true);

    PlayListInfo ret;
    try
    {
        ret = getDataFromRespones(response);
    }
    catch (const std::exception& e)
    {
        std::cout << "error: " << e.what() << std::endl;
        YOUTUBE_LOG_WARN("getVideoInfo error: {}", e.what());
    }
    return ret;
}

std::string YoutubeClient::getChannelUrl(const std::string& url)
{
    std::string channelUrl;
    LocationUrl response;
    get(url, response);

    if (!response.locationUrl.empty())
    {
        get(response.locationUrl, response);
    }

    struct XmlDocDeleter
    {
        void operator()(xmlDocPtr doc) const
        {
            xmlFreeDoc(doc);
        }
    };
    using XmlDocPtr = std::unique_ptr<xmlDoc, XmlDocDeleter>;
    struct XPathContextDeleter
    {
        void operator()(xmlXPathContextPtr ctx) const
        {
            xmlXPathFreeContext(ctx);
        }
    };
    using XPathContextPtr = std::unique_ptr<xmlXPathContext, XPathContextDeleter>;
    struct XPathObjectDeleter
    {
        void operator()(xmlXPathObjectPtr obj) const
        {
            xmlXPathFreeObject(obj);
        }
    };
    using XPathObjectPtr = std::unique_ptr<xmlXPathObject, XPathObjectDeleter>;
    struct XmlCharDeleter
    {
        void operator()(xmlChar* obj) const
        {
            xmlFree(obj);
        }
    };
    using XmlCharPtr = std::unique_ptr<xmlChar, XmlCharDeleter>;
    XmlDocPtr doc(htmlReadMemory(response.body.c_str(), response.body.size(), nullptr, nullptr, HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING));
    if (!doc)
    {
        return channelUrl;
    }
    XPathContextPtr ctx(xmlXPathNewContext(doc.get()));
    if (!ctx)
    {
        return channelUrl;
    }
    std::string expr = "//meta[@property='og:url']/@content";
    XPathObjectPtr result(xmlXPathEvalExpression((const xmlChar*)expr.c_str(), ctx.get()));
    if (!result)
    {
        return channelUrl;
    }
    if (result->nodesetval && result->nodesetval->nodeNr > 0)
    {
        XmlCharPtr content(xmlNodeListGetString(doc.get(), result->nodesetval->nodeTab[0]->xmlChildrenNode, 1));
        if (content)
        {
            channelUrl = std::string((const char*)content.get());
        }
    }
    return channelUrl;
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
        m_commonOptions[network::CookieFields::opt] = std::make_shared<network::CookieFields>(m_cookies.cookie(domain));
    }
}

}  // namespace youtubeapi
