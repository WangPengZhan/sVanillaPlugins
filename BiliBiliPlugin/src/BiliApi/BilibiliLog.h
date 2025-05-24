#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/common.h>

const std::string biliModuleName = "BiliBili";

#define BILIBILI_LOG_TRACE(...)    SPDLOG_LOGGER_TRACE(spdlog::get(biliModuleName), __VA_ARGS__)
#define BILIBILI_LOG_DEBUG(...)    SPDLOG_LOGGER_DEBUG(spdlog::get(biliModuleName), __VA_ARGS__)
#define BILIBILI_LOG_INFO(...)     SPDLOG_LOGGER_INFO(spdlog::get(biliModuleName), __VA_ARGS__)
#define BILIBILI_LOG_WARN(...)     SPDLOG_LOGGER_WARN(spdlog::get(biliModuleName), __VA_ARGS__)
#define BILIBILI_LOG_ERROR(...)    SPDLOG_LOGGER_ERROR(spdlog::get(biliModuleName), __VA_ARGS__)
#define BILIBILI_LOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(spdlog::get(biliModuleName), __VA_ARGS__)
#define BILIBILI_LOG_OFF(...)      SPDLOG_LOGGER_OFF(spdlog::get(biliModuleName), __VA_ARGS__)
