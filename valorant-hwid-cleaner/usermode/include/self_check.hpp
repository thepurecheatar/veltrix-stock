// self_check.hpp — single-instance mutex + abort-on-exception.
#pragma once
#include "common.hpp"

namespace jc::self {

// takes a global named mutex; returns false if another instance is already running.
bool acquire_single_instance();

// installs an unhandled-exception filter so we at least dump a line to the log before crashing.
void install_crash_handler();

} // namespace jc::self
