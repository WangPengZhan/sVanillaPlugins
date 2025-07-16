#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/common.h>
inline const std::string neteaseCloudMusicMuleName = "NeteaseCloudMusic";

#define NETEASE_LOG_TRACE(...)    SPDLOG_LOGGER_TRACE(spdlog::get(neteaseCloudMusicMuleName), __VA_ARGS__)
#define NETEASE_LOG_DEBUG(...)    SPDLOG_LOGGER_DEBUG(spdlog::get(neteaseCloudMusicMuleName), __VA_ARGS__)
#define NETEASE_LOG_INFO(...)     SPDLOG_LOGGER_INFO(spdlog::get(neteaseCloudMusicMuleName), __VA_ARGS__)
#define NETEASE_LOG_WARN(...)     SPDLOG_LOGGER_WARN(spdlog::get(neteaseCloudMusicMuleName), __VA_ARGS__)
#define NETEASE_LOG_ERROR(...)    SPDLOG_LOGGER_ERROR(spdlog::get(neteaseCloudMusicMuleName), __VA_ARGS__)
#define NETEASE_LOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(spdlog::get(neteaseCloudMusicMuleName), __VA_ARGS__)
#define NETEASE_LOG_OFF(...)      SPDLOG_LOGGER_OFF(spdlog::get(neteaseCloudMusicMuleName), __VA_ARGS__)
