// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_CORE_EXECUTION_TIMER_H
#define JACTORIO_INCLUDE_CORE_EXECUTION_TIMER_H
#pragma once

#include <chrono>
#include <map>
#include <mutex>
#include <string>

#define EXECUTION_PROFILE_SCOPE(timer_variable_name, item_name) \
    auto(timer_variable_name) = jactorio::core::ExecutionTimer(item_name)

#define EXECUTION_PROFILE_SCOPE_STOP(timer_variable_name) timer_variable_name.stop()

namespace jactorio::core
{
    ///
    /// Times execution time within a scope, to stop early, call stop() <br>
    /// Begins counter upon instantiation, stops upon being destroyed or stop called
    class ExecutionTimer
    {
    public:
        explicit ExecutionTimer(const std::string& name);
        ~ExecutionTimer();

        ///
        /// Stops the Execution_timer
        void Stop() const noexcept;

        ExecutionTimer(const ExecutionTimer& other)     = delete;
        ExecutionTimer(ExecutionTimer&& other) noexcept = delete;
        ExecutionTimer& operator=(const ExecutionTimer& other) = delete;
        ExecutionTimer& operator=(ExecutionTimer&& other) noexcept = delete;


        ///
        /// Contains measured times of all Execution_timers <br>
        /// Key is the timer_name_ of each instance
        /// Value is time taken in milliseconds
        static std::map<std::string, double> measuredTimes;

    private:
        mutable std::mutex measuredTimesMutex_ = std::mutex{};

        // Name of item being timed, used for tracking timers
        std::string timerName_;
        std::chrono::time_point<std::chrono::high_resolution_clock> startTime_;
    };
} // namespace jactorio::core

#endif // JACTORIO_INCLUDE_CORE_EXECUTION_TIMER_H
