#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/common.h>

const std::string biliModuleName = "BiliBili";

#define BILIBILI_LOG_TRACE(...)    SPDLOG_LOGGER_CALL(spdlog::get(biliModuleName), spdlog::level::trace, __VA_ARGS__)
#define BILIBILI_LOG_DEBUG(...)    SPDLOG_LOGGER_CALL(spdlog::get(biliModuleName), spdlog::level::debug, __VA_ARGS__)
#define BILIBILI_LOG_INFO(...)     SPDLOG_LOGGER_CALL(spdlog::get(biliModuleName), spdlog::level::info, __VA_ARGS__)
#define BILIBILI_LOG_WARN(...)     SPDLOG_LOGGER_CALL(spdlog::get(biliModuleName), spdlog::level::warn, __VA_ARGS__)
#define BILIBILI_LOG_ERROR(...)    SPDLOG_LOGGER_CALL(spdlog::get(biliModuleName), spdlog::level::err, __VA_ARGS__)
#define BILIBILI_LOG_CRITICAL(...) SPDLOG_LOGGER_CALL(spdlog::get(biliModuleName), spdlog::level::critical, __VA_ARGS__)
#define BILIBILI_LOG_OFF(...)      SPDLOG_LOGGER_CALL(spdlog::get(biliModuleName), spdlog::level::off, __VA_ARGS__)
