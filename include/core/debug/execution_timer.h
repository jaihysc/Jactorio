#ifndef CORE_DEBUG_EXECUTION_TIMER_H
#define CORE_DEBUG_EXECUTION_TIMER_H

#include <chrono>
#include <string>
#include <map>
#include <mutex>

#define EXECUTION_PROFILE_SCOPE(timer_variable_name, item_name) \
	auto (timer_variable_name) = jactorio::core::Execution_timer(item_name)

#define EXECUTION_PROFILE_SCOPE_STOP(timer_variable_name) \
	timer_variable_name.stop()

namespace jactorio::core
{
	/**
	 * Times execution time within a scope, to stop early, call stop() <br>
	 * Begins counter upon instantiation, stops upon being destroyed or stop called
	 */
	class Execution_timer
	{
	public:
		/**
		 * Contains measured times of all Execution_timers <br>
		 * Key is the timer_name_ of each instance
		 * Value is time taken in milliseconds
		 */
		static std::map<std::string, double> measured_times;
		
	private:
		static std::mutex measured_times_mutex_;
		
		// Name of item being timed, used for tracking timers
		std::string timer_name_;
		std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;

	public:
		Execution_timer(const std::string& name);
		
		~Execution_timer();

		/**
		 * Stops the Execution_timer
		 */
		void stop() const noexcept;

		Execution_timer(const Execution_timer& other) = delete;
		Execution_timer(Execution_timer&& other) noexcept = delete;
		Execution_timer& operator=(const Execution_timer& other) = delete;
		Execution_timer& operator=(Execution_timer&& other) noexcept = delete;
	};
}

#endif // CORE_DEBUG_EXECUTION_TIMER_H
