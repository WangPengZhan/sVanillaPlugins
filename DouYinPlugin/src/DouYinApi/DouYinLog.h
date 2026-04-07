#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/common.h>

const std::string douyinModuleName = "DouYin";

#define DOUYIN_LOG_TRACE(...)    SPDLOG_LOGGER_TRACE(spdlog::get(douyinModuleName), __VA_ARGS__)
#define DOUYIN_LOG_DEBUG(...)    SPDLOG_LOGGER_DEBUG(spdlog::get(douyinModuleName), __VA_ARGS__)
#define DOUYIN_LOG_INFO(...)     SPDLOG_LOGGER_INFO(spdlog::get(douyinModuleName), __VA_ARGS__)
#define DOUYIN_LOG_WARN(...)     SPDLOG_LOGGER_WARN(spdlog::get(douyinModuleName), __VA_ARGS__)
#define DOUYIN_LOG_ERROR(...)    SPDLOG_LOGGER_ERROR(spdlog::get(douyinModuleName), __VA_ARGS__)
#define DOUYIN_LOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(spdlog::get(douyinModuleName), __VA_ARGS__)
#define DOUYIN_LOG_OFF(...)      SPDLOG_LOGGER_OFF(spdlog::get(douyinModuleName), __VA_ARGS__)
