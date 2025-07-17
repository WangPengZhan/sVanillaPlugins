#include <iostream>
#include <string>

#include <gtest/gtest.h>

#include <PluginCommon/BaseVideoView.h>

#include "TemplatePluginCall.h"
#include "NeteaseCloudMusicApi/NeteaseCloudMusicClient.h"
#include "NeteaseCloudMusicApi/NeteaseCloudMusicUtils.h"
#include "NeteaseCloudMusicApi/NeteaseCloudMusicConstants.h"
#include "NeteaseCloudMusicApi/NeteaseCloudMusicApi.h"

namespace
{
void formatPluginMessage(const PluginMessage& message)
{
    std::cout << "Plugin ID: " << message.pluginId << std::endl;
    std::cout << "Name: " << message.name << std::endl;
    std::cout << "Version: " << message.version << std::endl;
    std::cout << "Description: " << message.description << std::endl;
    std::cout << "Domain: " << message.domain << std::endl;
}

void formatVideoView(const adapter::VideoView& views)
{
    std::cout << "views size: " << views.size() << std::endl;
    for (const auto& view : views)
    {
        std::cout << "Video ID: " << view.VideoId << std::endl;
        std::cout << "Title: " << view.Title << std::endl;
        std::cout << "Publisher: " << view.Publisher << std::endl;
        std::cout << "Cover: " << view.Cover << std::endl;
        std::cout << "Duration: " << view.Duration << std::endl;
        std::cout << "Description: " << view.Description << std::endl;
        std::cout << "Publish Date: " << view.PublishDate << std::endl;
        std::cout << "Playlist Title: " << view.PlayListTitle << std::endl;
        std::cout << "File Type: " << static_cast<int>(view.fileType) << std::endl;
    }
}
}  // namespace

TEST(NeteaseCloudMusic, weapi)
{
    constexpr char weapi_input[] =
        R"({"username":"NTE4MzU3M0U0NkRCRUE3MzcwRTEwQTlFQThENjIzNUVEQzJCMUU3MDc0RUZBNzI5QzhFQw0gOVoybVpGTnpyczhZM2lHVDVuRFQydz09","csrf_token":""})";
    constexpr char result[] = "";
    auto reuslt = netease::weapi(weapi_input);
}

TEST(NeteaseCloudMusic, aesEncrypt)
{
    constexpr char weapi_input[] =
        R"({"username":"OUY0MTdGNkRFNDZFNkM2NkU0NjZDNUIzQTZBNTFGMjZFMUU5M0E0ODFBNDAwMjk3NjY3QQ0gbEkyL1Q1akk1aGw2TlJsdGhOdTBlZz09","csrf_token":""})";
    constexpr char key[] = "0CoJUm6Qyw8W8jud";
    constexpr char iv[] = "0102030405060708";
    constexpr char mode[] = "cbc";
    constexpr char expact[] = "rNw9p+FHsMQJby9FopcVf8JURqwae8XTuqQ1KJwcBqW/fwOpmeV4vfohFGA93q/3uq+F6GMe0XKPJ7M+1DNvXmkzFWzWnmoDto0eH5L4E0ZVG6F+7gJo39K/"
                              "JN7HZFDkyHHJBWJS8wyxdcB7l52arQ2LkOoVVWW7wq4EuvR3LJ2NRyeCIO8YASxS9HB33ZCc";
    constexpr char secretKey[] = "ICcMJJu5PDHi502D";
    auto reuslt = netease::aesEncrypt(weapi_input, mode, key, iv, "base64");
    EXPECT_EQ(reuslt, expact);

    constexpr char second_expact[] =
        "rC7cWyXQUtPylKLg6gpCSYyzgHI72Bh4ZTEgkbgHrV+1vVgsLWjmayXfqsvRJzx/"
        "Z0Yf6ZPV59j18hUaXxxoNUw574Q4ZiGTHbFEFNywi0Ykfk5MRKsJI0B+TLtiCrM7xBbfix15rpk2wH+8vCPcGxEylDNLCBgRe4WulZqzdZI3aVTf8HbOdqID5aNdOzZsmHjt3zlN/"
        "+di+gcZHMv1gbfqLInomW3aPfm1e7G/bDr0bbW7nDGb1UIMUpGqcAVx8rxu0o0fPPPczUmR+mBZUw==";
    reuslt = netease::aesEncrypt(reuslt, mode, secretKey, iv, "base64");
    EXPECT_EQ(reuslt, second_expact);
}

TEST(NeteaseCloudMusic, cloudMusicDllEncodeID)
{
    constexpr char id[] = R"(5EB26EA49AE37590BF1618A40D227A1E286F98F4AF0F89AE4565)";
    constexpr char expact[] = "n2ajvcolCXIBkynlYKPukA==";
    auto reuslt = netease::cloudMusicDllEncodeID(id);
    EXPECT_EQ(reuslt, expact);
}

TEST(NeteaseCloudMusic, getAnonimousEncodeID)
{
    constexpr char id[] = R"(5EB26EA49AE37590BF1618A40D227A1E286F98F4AF0F89AE4565)";
    constexpr char expact[] = "NUVCMjZFQTQ5QUUzNzU5MEJGMTYxOEE0MEQyMjdBMUUyODZGOThGNEFGMEY4OUFFNDU2NSBuMmFqdmNvbENYSUJreW5sWUtQdWtBPT0=";
    auto digestHex = netease::cloudMusicDllEncodeID(id);
    auto reuslt = netease::base64Encode(std::string(id) + " " + digestHex);
    EXPECT_EQ(reuslt, expact);
}

TEST(NeteaseCloudMusic, rsaEncrypt)
{
    constexpr char id[] = R"(M7HYlilT7RacVS23)";
    constexpr char expact[] =
        "5ea22ed5c6dad3304a214bba58866ddd1ec0e1d5f661243abcd7cccc54a49839393d7a225248fddbe03cf5ce7a696f156bf6aad6500c637cce63bacdacc0960e238e6370e758f5ab36e54f"
        "ec2ac5be5fde5f2e4fbb253a2a0d1104a0093f0623984a0150cd249fa5d981470e6694d7c49ccb3f1052a01a9314ff5d06ff5aa4c1";
    std::string reversedKey = id;
    std::ranges::reverse(reversedKey);
    auto reuslt = netease::rsaEncrypt(reversedKey, netease::Encrypt::PUBLIC_KEY);
    EXPECT_EQ(reuslt, expact);
}

TEST(NeteaseCloudMusic, registerAnonimous)
{
    netease::NeteaseCloudMusicClient client;
    client.registerAnonimous();
}

TEST(NeteaseCloudMusic, getLoginKey)
{
    netease::NeteaseCloudMusicClient client;
    auto key = client.getLoginKey();
    auto status = client.getLoginStatus(key.unikey);
}

TEST(NeteaseCloudMusic, getSongDetails)
{
    netease::NeteaseCloudMusicClient client;
    client.registerAnonimous();
    client.getSongDetails({347230});
    // client.getSongPlayUrl({347230});
}

TEST(NeteaseCloudMusic, getAblum)
{
    netease::NeteaseCloudMusicClient client;
    client.registerAnonimous();
    client.getAblum("28521");
}

TEST(NeteaseCloudMusic, getPlaylist)
{
    netease::NeteaseCloudMusicClient client;
    client.registerAnonimous();
    client.getMVDetail("5436712");
    client.getMVPlayUrl("5436712");
}
