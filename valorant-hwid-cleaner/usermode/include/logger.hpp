// logger.hpp — console + file logger, levels, thread-safe. C++17, Win11 x64.
#pragma once
#include "common.hpp"
#include <mutex>
#include <chrono>
#include <fstream>
#include <sstream>

namespace jc::log {

enum class lvl { trace, info, ok, warn, err, step };

class logger {
    std::mutex m_;
    std::ofstream f_;
    HANDLE con_{};
public:
    logger();
    ~logger();
    void open(const std::filesystem::path& p);
    void line(lvl l, const std::string& s);
};

logger& g();

template <typename... A>
inline std::string fmt(const char* fmtstr, A&&... a) {
    char buf[1024];
    int n = _snprintf_s(buf, sizeof(buf), _TRUNCATE, fmtstr, std::forward<A>(a)...);
    return (n > 0) ? std::string(buf, n) : std::string{};
}

#define JC_LOG(LEVEL, ...) ::jc::log::g().line(::jc::log::lvl::LEVEL, ::jc::log::fmt(__VA_ARGS__))

} // namespace jc::log
