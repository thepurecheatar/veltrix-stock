// self_check.cpp — hardening.
#include "self_check.hpp"
#include "logger.hpp"

namespace jc::self {

static HANDLE g_mtx = nullptr;

bool acquire_single_instance() {
    g_mtx = CreateMutexW(nullptr, TRUE, L"Global\\JailCrashHwidCleaner_6E2F");
    if (!g_mtx) return false;
    DWORD e = GetLastError();
    if (e == ERROR_ALREADY_EXISTS) {
        CloseHandle(g_mtx);
        g_mtx = nullptr;
        return false;
    }
    return true;
}

static LONG WINAPI crash_filter(EXCEPTION_POINTERS* ep) {
    JC_LOG(err, "fatal exception: code=%08x addr=%p",
           ep->ExceptionRecord->ExceptionCode,
           ep->ExceptionRecord->ExceptionAddress);
    return EXCEPTION_EXECUTE_HANDLER;
}

void install_crash_handler() {
    SetUnhandledExceptionFilter(crash_filter);
}

} // namespace jc::self
