// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 12/06/2019

#include "core/execution_timer.h"

std::map<std::string, double>
jactorio::core::ExecutionTimer::measuredTimes = std::map<std::string, double>();

jactorio::core::ExecutionTimer::ExecutionTimer(const std::string& name) {
	timerName_ = name;
	startTime_ = std::chrono::high_resolution_clock::now();
}

jactorio::core::ExecutionTimer::~ExecutionTimer() {
	Stop();
}

void jactorio::core::ExecutionTimer::Stop() const noexcept {
	const long long start =
		std::chrono::time_point_cast<std::chrono::microseconds>(startTime_)
		.time_since_epoch().count();

	const auto end_time = std::chrono::high_resolution_clock::now();
	const long long end =
		std::chrono::time_point_cast<std::chrono::microseconds>(end_time)
		.time_since_epoch().count();

	// Microseconds conversion to milliseconds
	const double milliseconds = (end - start) * 0.001f;

	std::lock_guard guard(measuredTimesMutex_);
	try {
		measuredTimes[timerName_] = milliseconds;
	}
	catch (std::bad_alloc&) {
	}
}
