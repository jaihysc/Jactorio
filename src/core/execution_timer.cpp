// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "core/execution_timer.h"

#include "core/convert.h"

using namespace jactorio;

std::map<std::string, double> ExecutionTimer::measuredTimes = std::map<std::string, double>();

ExecutionTimer::ExecutionTimer(const std::string& name) {
    timerName_ = name;
    startTime_ = std::chrono::high_resolution_clock::now();
}

ExecutionTimer::~ExecutionTimer() {
    Stop();
}

void ExecutionTimer::Stop() const noexcept {
    const auto start = std::chrono::time_point_cast<std::chrono::microseconds>(startTime_).time_since_epoch().count();

    const auto end_time = std::chrono::high_resolution_clock::now();
    const auto end      = std::chrono::time_point_cast<std::chrono::microseconds>(end_time).time_since_epoch().count();

    // Microseconds conversion to milliseconds
    const double milliseconds = SafeCast<double>(end - start) * 0.001;

    std::lock_guard guard(measuredTimesMutex_);
    try {
        measuredTimes[timerName_] = milliseconds;
    }
    catch (std::bad_alloc&) {
    }
}
