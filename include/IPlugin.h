#pragma once
#include <atomic>
#include <string>

namespace Plugin
{
class IPlugin
{
public:
  virtual ~IPlugin() = default;

  [[nodiscard]] virtual const std::string &pluginName() const = 0;
  [[nodiscard]] virtual const std::string &pluginVersion() const = 0;
};

} // namespace Plugin
