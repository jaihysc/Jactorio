#ifndef DATA_PROTOTYPE_PROTOTYPE_BASE_H
#define DATA_PROTOTYPE_PROTOTYPE_BASE_H

#include <string>

#include "data/data_category.h"

namespace jactorio::data
{
	// Creates a setters for python API primarily, to chain initialization

	// Setter passed by reference
#define PYTHON_PROP_REF(class_, type, var_name) type var_name; \
	class_* set_##var_name(const type& (var_name)) {\
		this->var_name = var_name;\
		return this;\
	}

	// Setter passed by value
#define PYTHON_PROP(class_, type, var_name) type var_name; \
	class_* set_##var_name(type (var_name)) {\
		this->var_name = var_name;\
		return this;\
	}
	
	/*
		The following is an example:

		PYTHON_PROP_REF(Prototype_base, unsigned int, internal_id)

		vvv

		unsigned int internal_id = 0;

		Prototype_base* set_internal_id(const unsigned int& internal_id) {
			this->internal_id = internal_id;
			return this;
		}
	*/
	
	class Prototype_base
	{
	public:
		Prototype_base()
			: internal_id(0), category(data_category::none), order(0) {
		};

		// This is needed so pybind can return the right type on .add()
		virtual ~Prototype_base() = default;


		Prototype_base(const Prototype_base& other) = default;
		Prototype_base(Prototype_base&& other) noexcept = default;
		Prototype_base& operator=(const Prototype_base& other) = default;
		Prototype_base& operator=(Prototype_base&& other) noexcept = default;
		
		/**
		 * Unique per prototype, auto assigned per new prototype added, faster than
		 * std::string name <br>J
		 * 0 indicates invalid id
		 */
		PYTHON_PROP_REF(Prototype_base, unsigned int, internal_id)
		
		/**
		 * Internal name <br>
		 * MUST BE unique per data_category
		 */
		PYTHON_PROP_REF(Prototype_base, std::string, name)

		/**
		 * Category of this Prototype item
		 */
		PYTHON_PROP_REF(Prototype_base, data_category, category)

		/**
		 * Determines the priority of this prototype, used in certain situations, see documentation
		 * within inheritors <br<
		 * Automatically assigned incrementally alongside internal_id if not defined <br>
		 * 0 indicates invalid id
		 */
		PYTHON_PROP_REF(Prototype_base, unsigned int, order)


		// Localized
		std::string localized_name;
	};
}

#endif // DATA_PROTOTYPE_PROTOTYPE_BASE_H
