#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/common.h>
const std::string youtubeModuleName = "Youtube";

#define YOUTUBE_LOG_TRACE(...)    SPDLOG_LOGGER_TRACE(spdlog::get(youtubeModuleName), __VA_ARGS__)
#define YOUTUBE_LOG_DEBUG(...)    SPDLOG_LOGGER_DEBUG(spdlog::get(youtubeModuleName), __VA_ARGS__)
#define YOUTUBE_LOG_INFO(...)     SPDLOG_LOGGER_INFO(spdlog::get(youtubeModuleName), __VA_ARGS__)
#define YOUTUBE_LOG_WARN(...)     SPDLOG_LOGGER_WARN(spdlog::get(youtubeModuleName), __VA_ARGS__)
#define YOUTUBE_LOG_ERROR(...)    SPDLOG_LOGGER_ERROR(spdlog::get(youtubeModuleName), __VA_ARGS__)
#define YOUTUBE_LOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(spdlog::get(youtubeModuleName), __VA_ARGS__)
#define YOUTUBE_LOG_OFF(...)      SPDLOG_LOGGER_OFF(spdlog::get(youtubeModuleName), __VA_ARGS__)
