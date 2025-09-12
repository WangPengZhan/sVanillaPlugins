#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/common.h>

const std::string dedaoModuleName = "Dedao";

#define DEDAO_LOG_TRACE(...)    SPDLOG_LOGGER_TRACE(spdlog::get(dedaoModuleName), __VA_ARGS__)
#define DEDAO_LOG_DEBUG(...)    SPDLOG_LOGGER_DEBUG(spdlog::get(dedaoModuleName), __VA_ARGS__)
#define DEDAO_LOG_INFO(...)     SPDLOG_LOGGER_INFO(spdlog::get(dedaoModuleName), __VA_ARGS__)
#define DEDAO_LOG_WARN(...)     SPDLOG_LOGGER_WARN(spdlog::get(dedaoModuleName), __VA_ARGS__)
#define DEDAO_LOG_ERROR(...)    SPDLOG_LOGGER_ERROR(spdlog::get(dedaoModuleName), __VA_ARGS__)
#define DEDAO_LOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(spdlog::get(dedaoModuleName), __VA_ARGS__)
#define DEDAO_LOG_OFF(...)      SPDLOG_LOGGER_OFF(spdlog::get(dedaoModuleName), __VA_ARGS__)
