#include <gmock/gmock.h>
#include "YoutubePlugin.h"

using namespace testing;

class YoutubePluginTest : public Test
{
public:
    YoutubePlugin *plugin = new YoutubePlugin;

    void SetUp() override
    {
    }
    void TearDown() override
    {
        delete plugin;
    }
};

TEST_F(YoutubePluginTest, pluginName)
{
    ASSERT_EQ(plugin->pluginName(), "youtube");
}
