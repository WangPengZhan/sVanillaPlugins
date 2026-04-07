#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

#include <NetWork/CNetWork.h>
#include <NetWork/CurlCpp/CurlCookies.h>

#include "XHSApi.h"

namespace xhsapi
{

class XHSClient : public network::NetWork
{
public:
    XHSClient();
    ~XHSClient() = default;

    static XHSClient& globalClient();

    QRCodeResponse getQRCode();
    LoginStatusResponse getLoginStatus(const std::string& code, const std::string& qrId);
    AccountInfoResponse getAccountInfo();

    NoteDetailResponse getNoteDetail(const std::string& noteId, const std::string& xsecToken, const std::string& xsecSource = "pc_feed");
    NoteItemListResponse getAccountNotes(const std::string& userId, int cursor = 0, int num = 10, const std::string& xsecToken = "",
                                         const std::string& xsecSource = "");
    NoteItemListResponse getFavoriteNotes(const std::string& userId, int cursor = 0, int num = 10, const std::string& xsecToken = "",
                                          const std::string& xsecSource = "");

    bool isLogined() const;
    std::string cookies() const;
    void setCookies(std::string cookies);

private:
    void initDefaultOptions();
    void registerAnonymous();

    void setExploreCookie();
    void setScriptingCookie();
    void setActivateCookie();

private:
    static network::CurlHeader createLoginHeaders();
    static std::string encodeData(const ParamType& params);
    static nlohmann::json getDataFromRespones(const std::string& respones);

private:
    network::CurlCookies m_cookies;
};

}  // namespace xhsapi
