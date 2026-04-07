#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/common.h>

const std::string xhsModuleName = "XHS";

#define XHS_LOG_TRACE(...)    SPDLOG_LOGGER_TRACE(spdlog::get(xhsModuleName), __VA_ARGS__)
#define XHS_LOG_DEBUG(...)    SPDLOG_LOGGER_DEBUG(spdlog::get(xhsModuleName), __VA_ARGS__)
#define XHS_LOG_INFO(...)     SPDLOG_LOGGER_INFO(spdlog::get(xhsModuleName), __VA_ARGS__)
#define XHS_LOG_WARN(...)     SPDLOG_LOGGER_WARN(spdlog::get(xhsModuleName), __VA_ARGS__)
#define XHS_LOG_ERROR(...)    SPDLOG_LOGGER_ERROR(spdlog::get(xhsModuleName), __VA_ARGS__)
#define XHS_LOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(spdlog::get(xhsModuleName), __VA_ARGS__)
#define XHS_LOG_OFF(...)      SPDLOG_LOGGER_OFF(spdlog::get(xhsModuleName), __VA_ARGS__)
