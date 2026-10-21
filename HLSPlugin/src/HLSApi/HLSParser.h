#pragma once
#include <unordered_map>
#include <string_view>

#include "HLSApi.h"

namespace hlsapi
{

class HLSParser
{
public:
    bool parse(std::string_view content);
    void parseLine(std::string_view line);

    const M3U8Master& master() const;
    bool isMaster() const;

    const M3U8Playlist& playlist() const;
    const std::vector<std::string>& unparsedTags() const;

    void setOriginUri(std::string uri);
    const std::string& originUri() const;

private:
    enum State
    {
        None,
        WaitingForStreamUri,
        WaitingForSegmentUri
    };
    State m_state = State::None;

    std::string m_originUri;
    std::string m_baseUri;
    M3U8Playlist m_playlist;
    M3U8Master m_master;
    KeyInfo m_activeKey;
    std::vector<std::string> m_unparsedTags;
    std::string m_previousRangeUri;
    std::size_t m_previousRangeEnd{};
    bool m_hasPreviousRange{};
    bool m_valid{true};
};

}  // namespace hlsapi
