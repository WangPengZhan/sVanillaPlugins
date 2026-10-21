#pragma once

#include <string>

#include "NetWork/CurlCpp/CurlResponseWrapper.h"
#include "NetWork/CurlCpp/CurlWriter.h"

namespace hlsapi
{
struct StringAndEffectiveUrl
{
    std::string body;
    std::string effectiveUrl;
};
}  // namespace hlsapi

namespace network
{
template <>
class CurlResponseWrapper<hlsapi::StringAndEffectiveUrl>
{
public:
    explicit CurlResponseWrapper(hlsapi::StringAndEffectiveUrl& response)
        : m_response(response)
    {
    }

    void setToCurl(CURL* handle)
    {
        if (handle)
        {
            curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
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
        char* effectiveUrl = nullptr;
        if (handle && curl_easy_getinfo(handle, CURLINFO_EFFECTIVE_URL, &effectiveUrl) == CURLE_OK && effectiveUrl)
        {
            m_response.effectiveUrl = effectiveUrl;
        }
    }

    void readAfter(CurlEasy& easy)
    {
        readAfter(easy.handle());
    }

private:
    hlsapi::StringAndEffectiveUrl& m_response;
};
}  // namespace network
