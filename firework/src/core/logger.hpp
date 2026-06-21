#pragma once
#include <string>
#include <format>
#include <chrono>
#include <print>
#include <source_location>
#include <filesystem>

#include "../firework.hpp"

namespace Firework
{
    
enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL,
};

inline auto to_string(LogLevel level) -> std::string {
    switch (level) {
        using enum LogLevel;
        case DEBUG:     return "DEBUG"; 
        case INFO:      return "INFO"; 
        case WARN:      return "WARN"; 
        case ERROR:     return "ERROR"; 
        case CRITICAL:  return "CRITICAL";
        default:        return "?";
    }
}

template<class... Args>
struct format_with_location {
    std::format_string<Args...> fmt;
    std::source_location loc;

    template<class T>
    consteval format_with_location(const T& f, std::source_location l = std::source_location::current())
        : fmt(f), loc(l) {}
};

class Logger {
public:
    Logger(const std::string &directory, const std::string &subDirectory);

    template <typename ..._Args>
    auto debug(std::format_string<_Args...> fmt, _Args &&...args) -> void;

    template <typename ..._Args>
    auto info(std::format_string<_Args...> fmt, _Args &&...args) -> void;

    template <typename ..._Args>
    auto warn(std::format_string<_Args...> fmt, _Args &&...args) -> void;

    template <typename ..._Args>
    auto error(
        format_with_location<std::type_identity_t<_Args>...> fmt, 
        _Args &&...args
    ) -> void;

    template <typename ..._Args>
    auto critical(
        format_with_location<std::type_identity_t<_Args>...> fmt, 
        _Args &&...args
    ) -> void;

private:
    std::string _directory; 
    std::string _subDirectory;

    template <typename... _Args>
    auto log(
        LogLevel level, 
        std::format_string<_Args...> fmt, 
        std::optional<const std::source_location> sourceLocation,
        _Args &&...args
    ) -> void;

    auto get_time_string() -> std::string;
};



template <typename ..._Args>
auto Logger::debug(std::format_string<_Args...> fmt, _Args &&...args) -> void {
    this->log(LogLevel::DEBUG, fmt, std::nullopt, std::forward<_Args>(args)...);
}

template <typename ..._Args>
auto Logger::info(std::format_string<_Args...> fmt, _Args &&...args) -> void {
    this->log(LogLevel::INFO, fmt, std::nullopt, std::forward<_Args>(args)...);
}

template <typename ..._Args>
auto Logger::warn(std::format_string<_Args...> fmt, _Args &&...args) -> void {
    this->log(LogLevel::WARN, fmt, std::nullopt, std::forward<_Args>(args)...);
}

template <typename ..._Args>
auto Logger::error(
    format_with_location<std::type_identity_t<_Args>...> fmt, 
    _Args &&...args
) -> void {
    this->log(LogLevel::ERROR, fmt.fmt, fmt.loc, std::forward<_Args>(args)...);
}

template <typename ..._Args>
auto Logger::critical(
    format_with_location<std::type_identity_t<_Args>...> fmt,
    _Args &&...args
) -> void {
    this->log(LogLevel::CRITICAL, fmt.fmt, fmt.loc, std::forward<_Args>(args)...);
}

template <typename... _Args>
auto Logger::log(
    LogLevel level, 
    std::format_string<_Args...> fmt,
    std::optional<const std::source_location> sourceLocation,
    _Args &&...args
) -> void {
    std::string timeStr = get_time_string();
    std::string formattedMsg = std::format(fmt, std::forward<_Args>(args)...);

#ifndef FIREWORK_RELEASE
    if (sourceLocation.has_value()) {
        std::filesystem::path file{sourceLocation->file_name()};
        std::println("[{}] [{}] ({}/{}) in {}({}:{}) (function {}): {}",
            timeStr, 
            to_string(level), 
            _directory, _subDirectory,
            file.filename().string(),
            sourceLocation->line(),
            sourceLocation->column(),
            sourceLocation->function_name(),
            formattedMsg
        );
    }
    else 
#endif
    {
        std::println("[{}] [{}] ({}/{}): {}", 
            timeStr, 
            to_string(level), 
            _directory, _subDirectory, 
            formattedMsg
        );
    }
}

} // namespace Firework

