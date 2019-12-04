#include "core/debug/execution_timer.h"

std::map<std::string, double> jactorio::core::Execution_timer::measured_times = 
	std::map<std::string, double>();
std::mutex jactorio::core::Execution_timer::measured_times_mutex_ = std::mutex{};

jactorio::core::Execution_timer::Execution_timer(const std::string& name) {
	timer_name_ = name;
	start_time_ = std::chrono::high_resolution_clock::now();
}

jactorio::core::Execution_timer::~Execution_timer() {
	stop();
}

void jactorio::core::Execution_timer::stop() const noexcept {
	const long long start =
		std::chrono::time_point_cast<std::chrono::microseconds>(start_time_)
		.time_since_epoch().count();

	const auto end_time = std::chrono::high_resolution_clock::now();
	const long long end =
		std::chrono::time_point_cast<std::chrono::microseconds>(end_time)
		.time_since_epoch().count();

	// Microseconds conversion to milliseconds
	const double milliseconds = (end - start) * 0.001f;

	std::lock_guard guard(measured_times_mutex_);
	try {
		measured_times[timer_name_] = milliseconds;
	}
	catch (std::bad_alloc&) {
	}
}
