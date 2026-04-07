#include "XHSPlugin.h"

#include "XHSApi/XHSApi.h"
#include "XHSApi/XHSClient.h"
#include "XHSApi/XHSUrl.h"
#include "XHSResource.h"
#include "XHSDownloader.h"
#include "Convert.h"
#include "XHSApi/XHSLog.h"
#include "XHSPluginMessage.h"

namespace xhsplugin
{

PluginMessage XHSPlugin::m_pluginMessage = {
    xhsplugin::pluginID, xhsplugin::name, xhsplugin::version, xhsplugin::description, xhsplugin::domain,
};

std::string XHSPlugin::m_dir;

XHSPlugin::XHSPlugin()
    : IPlugin()
    , m_client(xhsapi::XHSClient::globalClient())
{
}

const PluginMessage& XHSPlugin::pluginMessage() const
{
    return m_pluginMessage;
}

const std::vector<uint8_t>& XHSPlugin::websiteIcon()
{
    return qrc_website_icon;
}

bool XHSPlugin::canParseUrl(const std::string& url)
{
    return xhsapi::isValidUrl(url);
}

adapter::VideoView XHSPlugin::getVideoView(const std::string& url)
{
    XHS_LOG_INFO("getVideoView: {}", url);
    xhsapi::IDInfo id = xhsapi::getID(url);
    XHS_LOG_INFO("id( {} )", id.to_string());
    if (id.id.empty())
    {
        return {};
    }

    adapter::VideoView views;
    switch (id.type)
    {
    case xhsapi::IDType::NoteId:
    {
        xhsapi::NoteDetailResponse noteDetail = m_client.getNoteDetail(id.id, id.xsecToken);
        adapter::BaseVideoView view = convertNoteDetail(noteDetail.data.note_card);
        view.Option1 = id.xsecToken;
        views.push_back(view);
        return views;
    }
    case xhsapi::IDType::UserId:
    {
        xhsapi::NoteItemList noteList;
        int cursor = 0;
        do
        {
            noteList = m_client.getAccountNotes(id.id, cursor, 10, id.xsecToken).data;
            auto viewList = convertNoteDetail(noteList);
            views.insert(views.end(), viewList.begin(), viewList.end());
            cursor += 10;
        } while (noteList.has_more);

        return views;
    }
    default:
        break;
    }

    return views;
}

std::shared_ptr<download::FileDownloader> XHSPlugin::getDownloader(const VideoInfoFull& videoInfo)
{
    auto copyedVideoInfo = videoInfo;
    copyedVideoInfo.downloadConfig = std::make_shared<DownloadConfig>(*videoInfo.downloadConfig);
    copyedVideoInfo.videoView = std::make_shared<adapter::BaseVideoView>(*videoInfo.videoView);

    xhsapi::NoteDetailResponse noteDetail = m_client.getNoteDetail(copyedVideoInfo.videoView->Identifier, copyedVideoInfo.videoView->Option1);

    download::ResourceInfo info;
    info.videoUris.push_back(getVideoUrl(noteDetail.data.note_card.video.media));
    auto fileName = videoInfo.fileName(true);
    info.option.out = fileName;
    info.option.dir = videoInfo.downloadConfig->downloadDir;
    const std::list<std::string> h = {"Referer: https://www.xiaohongshu.com/", std::string("User-Agent: ") + network::chrome,
                                      "Origin: https://www.xiaohongshu.com", "Accept-Encoding: gzip, deflate, br, zstd",
                                      "Accept-Language: en,zh-CN;q=0.9,zh;q=0.8"};
    info.option.header = h;

    auto xhsDownlaoder = std::shared_ptr<download::XHSDownloader>(new download::XHSDownloader(info), download::freeDownloader);
    return xhsDownlaoder;
}

LoginProxy XHSPlugin::loginer()
{
    return LoginProxy(m_login);
}

void XHSPlugin::setDir(std::string dir)
{
    m_dir = dir;
}

const std::string& XHSPlugin::getDir()
{
    return m_dir;
}

}  // namespace xhsplugin
