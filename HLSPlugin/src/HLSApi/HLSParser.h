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

    void setOriginUri(std::string uri);
    const std::string& originUri() const;

private:
    enum State
    {
        None,
        WaitingForStreamUri
    };
    State m_state = State::None;

    int m_mediaSequence = 0;
    double m_targetDuration = 0.0;

    std::string m_originUri;
    std::string m_baseUri;
    M3U8Playlist m_playlist;
    M3U8Master m_master;
    PendingInfo* m_pendingInfo = nullptr;
    MediaSegment* m_pendingMediaSegment = nullptr;
};

}  // namespace hlsapi
