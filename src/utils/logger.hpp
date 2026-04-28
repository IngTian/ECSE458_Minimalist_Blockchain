#pragma once

// Modern fmt-style logging facade over spdlog.
//
// Two ways to use it:
//
//   1. Native fmt-style — preferred for any new code:
//        bc::log::info("transaction", "spent UTXO {} for {} coins", txid_hex, value);
//
//   2. Legacy printf-style via general_log() in log_utils.h — kept as a shim
//      around spdlog for backward compatibility with existing call sites.
//
// The first invocation lazily initializes a colored stdout sink with the level
// from utils/constants.h (LOG_LEVEL).

#include <spdlog/spdlog.h>

#include <string_view>

namespace bc::log {

// Initialize the default spdlog sink + level. Idempotent; called from each
// public log helper, so callers don't need to set anything up.
void ensure_initialized();

namespace detail {

inline spdlog::level::level_enum level_from_legacy(int legacy_level) noexcept {
    // Maps utils/constants.h LOG_DEBUG / LOG_INFO / LOG_ERROR to spdlog levels.
    switch (legacy_level) {
        case 0: return spdlog::level::debug;
        case 1: return spdlog::level::info;
        case 2: return spdlog::level::err;
        default: return spdlog::level::info;
    }
}

}  // namespace detail

template <typename... Args>
inline void log(int legacy_level, std::string_view scope, fmt::format_string<Args...> fmt, Args&&... args) {
    ensure_initialized();
    auto level = detail::level_from_legacy(legacy_level);
    if (!spdlog::default_logger_raw()->should_log(level)) return;
    spdlog::default_logger_raw()->log(
        level, "({}) ----> {}", scope, fmt::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
inline void debug(std::string_view scope, fmt::format_string<Args...> fmt, Args&&... args) {
    log(0, scope, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void info(std::string_view scope, fmt::format_string<Args...> fmt, Args&&... args) {
    log(1, scope, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void error(std::string_view scope, fmt::format_string<Args...> fmt, Args&&... args) {
    log(2, scope, fmt, std::forward<Args>(args)...);
}

}  // namespace bc::log
