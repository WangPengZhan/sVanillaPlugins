#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/common.h>
const std::string youtubeModuleName = "Youtube";

#define YOUTUBE_LOG_TRACE(...)    SPDLOG_LOGGER_CALL(spdlog::get(youtubeModuleName), spdlog::level::trace, __VA_ARGS__)
#define YOUTUBE_LOG_DEBUG(...)    SPDLOG_LOGGER_CALL(spdlog::get(youtubeModuleName), spdlog::level::debug, __VA_ARGS__)
#define YOUTUBE_LOG_INFO(...)     SPDLOG_LOGGER_CALL(spdlog::get(youtubeModuleName), spdlog::level::info, __VA_ARGS__)
#define YOUTUBE_LOG_WARN(...)     SPDLOG_LOGGER_CALL(spdlog::get(youtubeModuleName), spdlog::level::warn, __VA_ARGS__)
#define YOUTUBE_LOG_ERROR(...)    SPDLOG_LOGGER_CALL(spdlog::get(youtubeModuleName), spdlog::level::err, __VA_ARGS__)
#define YOUTUBE_LOG_CRITICAL(...) SPDLOG_LOGGER_CALL(spdlog::get(youtubeModuleName), spdlog::level::critical, __VA_ARGS__)
#define YOUTUBE_LOG_OFF(...)      SPDLOG_LOGGER_CALL(spdlog::get(youtubeModuleName), spdlog::level::off, __VA_ARGS__)
