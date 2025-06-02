#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/common.h>

const std::string HLSModuleName = "HLS";

#define HLS_LOG_TRACE(...)    SPDLOG_LOGGER_CALL(spdlog::get(HLSModuleName), spdlog::level::trace, __VA_ARGS__)
#define HLS_LOG_DEBUG(...)    SPDLOG_LOGGER_CALL(spdlog::get(HLSModuleName), spdlog::level::debug, __VA_ARGS__)
#define HLS_LOG_INFO(...)     SPDLOG_LOGGER_CALL(spdlog::get(HLSModuleName), spdlog::level::info, __VA_ARGS__)
#define HLS_LOG_WARN(...)     SPDLOG_LOGGER_CALL(spdlog::get(HLSModuleName), spdlog::level::warn, __VA_ARGS__)
#define HLS_LOG_ERROR(...)    SPDLOG_LOGGER_CALL(spdlog::get(HLSModuleName), spdlog::level::err, __VA_ARGS__)
#define HLS_LOG_CRITICAL(...) SPDLOG_LOGGER_CALL(spdlog::get(HLSModuleName), spdlog::level::critical, __VA_ARGS__)
#define HLS_LOG_OFF(...)      SPDLOG_LOGGER_CALL(spdlog::get(HLSModuleName), spdlog::level::off, __VA_ARGS__)
