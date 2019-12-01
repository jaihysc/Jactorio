#ifndef CORE_RESOURCE_GUARD_H
#define CORE_RESOURCE_GUARD_H

namespace jactorio::core
{
	/**
	 * RAII wrapper which calls the provided function in its destructor
	 */
	template <typename T>
	class Resource_guard
	{
		// Function returning void
		using function = T (*)();
		
		function f_;
	public:
		/**
		 * @param f Function which will be called upon exiting current scope
		 */
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
}
#endif // CORE_RESOURCE_GUARD_H
