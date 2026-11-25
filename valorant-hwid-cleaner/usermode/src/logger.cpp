// logger.cpp — impl for jc::log. C++17, MSVC.
#include "logger.hpp"
#include <ctime>
#include <iomanip>

namespace jc::log {

logger::logger() {
    con_ = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (con_ && GetConsoleMode(con_, &mode)) {
        SetConsoleMode(con_, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}

logger::~logger() { if (f_.is_open()) f_.close(); }

void logger::open(const std::filesystem::path& p) {
    std::lock_guard lk(m_);
    f_.open(p, std::ios::app);
}

static const char* color_of(lvl l) {
    switch (l) {
        case lvl::trace: return "\x1b[90m";
        case lvl::info:  return "\x1b[37m";
        case lvl::ok:    return "\x1b[92m";
        case lvl::warn:  return "\x1b[93m";
        case lvl::err:   return "\x1b[91m";
        case lvl::step:  return "\x1b[96m";
    }
    return "\x1b[0m";
}

static const char* tag_of(lvl l) {
    switch (l) {
        case lvl::trace: return "..";
        case lvl::info:  return "  ";
        case lvl::ok:    return "ok";
        case lvl::warn:  return "!!";
        case lvl::err:   return "XX";
        case lvl::step:  return ">>";
    }
    return "  ";
}

void logger::line(lvl l, const std::string& s) {
    std::lock_guard lk(m_);
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_s(&tm, &t);
    char ts[16];
    std::strftime(ts, sizeof ts, "%H:%M:%S", &tm);

    std::string out = fmt("%s[%s %s]\x1b[0m %s\n", color_of(l), ts, tag_of(l), s.c_str());
    if (con_) {
        DWORD w = 0;
        WriteConsoleA(con_, out.data(), (DWORD)out.size(), &w, nullptr);
    }
    if (f_.is_open()) {
        f_ << fmt("[%s %s] %s\n", ts, tag_of(l), s.c_str());
        f_.flush();
    }
}

logger& g() {
    static logger L;
    return L;
}

} // namespace jc::log
