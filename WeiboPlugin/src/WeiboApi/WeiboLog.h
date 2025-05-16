#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/common.h>

const std::string weiboModuleName = "Weibo";

#define WEIBO_LOG_TRACE(...)    SPDLOG_LOGGER_CALL(spdlog::get(weiboModuleName), spdlog::level::trace, __VA_ARGS__)
#define WEIBO_LOG_DEBUG(...)    SPDLOG_LOGGER_CALL(spdlog::get(weiboModuleName), spdlog::level::debug, __VA_ARGS__)
#define WEIBO_LOG_INFO(...)     SPDLOG_LOGGER_CALL(spdlog::get(weiboModuleName), spdlog::level::info, __VA_ARGS__)
#define WEIBO_LOG_WARN(...)     SPDLOG_LOGGER_CALL(spdlog::get(weiboModuleName), spdlog::level::warn, __VA_ARGS__)
#define WEIBO_LOG_ERROR(...)    SPDLOG_LOGGER_CALL(spdlog::get(weiboModuleName), spdlog::level::err, __VA_ARGS__)
#define WEIBO_LOG_CRITICAL(...) SPDLOG_LOGGER_CALL(spdlog::get(weiboModuleName), spdlog::level::critical, __VA_ARGS__)
#define WEIBO_LOG_OFF(...)      SPDLOG_LOGGER_CALL(spdlog::get(weiboModuleName), spdlog::level::off, __VA_ARGS__)
