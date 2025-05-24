#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/common.h>

const std::string weiboModuleName = "Weibo";

#define WEIBO_LOG_TRACE(...)    SPDLOG_LOGGER_TRACE(spdlog::get(weiboModuleName), __VA_ARGS__)
#define WEIBO_LOG_DEBUG(...)    SPDLOG_LOGGER_DEBUG(spdlog::get(weiboModuleName), __VA_ARGS__)
#define WEIBO_LOG_INFO(...)     SPDLOG_LOGGER_INFO(spdlog::get(weiboModuleName), __VA_ARGS__)
#define WEIBO_LOG_WARN(...)     SPDLOG_LOGGER_WARN(spdlog::get(weiboModuleName), __VA_ARGS__)
#define WEIBO_LOG_ERROR(...)    SPDLOG_LOGGER_ERROR(spdlog::get(weiboModuleName), __VA_ARGS__)
#define WEIBO_LOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(spdlog::get(weiboModuleName), __VA_ARGS__)
#define WEIBO_LOG_OFF(...)      SPDLOG_LOGGER_OFF(spdlog::get(weiboModuleName), __VA_ARGS__)
