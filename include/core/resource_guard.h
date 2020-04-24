// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 12/01/2019

#ifndef JACTORIO_INCLUDE_CORE_RESOURCE_GUARD_H
#define JACTORIO_INCLUDE_CORE_RESOURCE_GUARD_H
#pragma once

#include <functional>

namespace jactorio::core
{
	///
	/// RAII wrapper which calls the provided function in its destructor
	template <typename T>
	class Resource_guard
	{
		// Function returning T
		using function = T (*)();

		function f_;
	public:
		///
		/// \param f Function which will be called upon exiting current scope
		explicit Resource_guard(const function f)
			: f_(f) {
		}

		~Resource_guard() {
			f_();
		}


		Resource_guard(const Resource_guard& other) = delete;
		Resource_guard(Resource_guard&& other) noexcept = delete;
		Resource_guard& operator=(const Resource_guard& other) = delete;
		Resource_guard& operator=(Resource_guard&& other) noexcept = delete;
	};

	///
	/// Capturing RAII wrapper which calls the provided function in its destructor
	template <typename T = void()>
	class Capturing_guard
	{
		using function = std::function<T>;

		function f_;
	public:
		///
		/// \param f Function which will be called upon exiting current scope
		explicit Capturing_guard(const function f)
			: f_(f) {
		}

		~Capturing_guard() {
			f_();
		}


		Capturing_guard(const Capturing_guard& other) = delete;
		Capturing_guard(Capturing_guard&& other) noexcept = delete;
		Capturing_guard& operator=(const Capturing_guard& other) = delete;
		Capturing_guard& operator=(Capturing_guard&& other) noexcept = delete;
	};
}
#endif //JACTORIO_INCLUDE_CORE_RESOURCE_GUARD_H
