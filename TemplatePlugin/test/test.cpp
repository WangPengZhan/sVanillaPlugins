#include <string>

#include <gtest/gtest.h>

#include "src/TemplatePluginCall.h"

TEST(FF_Test, INFO)
{
    std::cout << "test";
}


TEST(AAA_Test, INFO)
{
    std::string name = pluginName();
    EXPECT_EQ(name, "template");
    std::string version = pluginVersion();
    EXPECT_EQ(version, "1.0.0");
    EXPECT_EQ(pluginID(), 0);
    std::string decs = pluginDescription();
    EXPECT_EQ(decs, "description template");
}
