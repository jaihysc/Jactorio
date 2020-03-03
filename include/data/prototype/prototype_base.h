//
// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package
//

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_PROTOTYPE_BASE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_PROTOTYPE_BASE_H

#include <string>

#include "jactorio.h"

#include "data/data_category.h"
#include "data/data_exception.h"

namespace jactorio::data
{
	// Creates a setters for python API primarily, to chain initialization

	// Setter passed by reference
#define PYTHON_PROP_REF(class_, type, var_name) \
	type var_name; \
	class_* set_##var_name(const type& (var_name)) {\
		this->var_name = var_name;\
		return this;\
	}

	// Setter passed by reference with initializer
#define PYTHON_PROP_REF_I(class_, type, var_name, initializer) \
	type var_name = initializer; \
	class_* set_##var_name(const type& (var_name)) {\
		this->var_name = var_name;\
		return this;\
	}

	
	// Setter passed by value
#define PYTHON_PROP(class_, type, var_name) \
	type var_name; \
	class_* set_##var_name(type (var_name)) {\
		this->var_name = var_name;\
		return this;\
	}

	// Setter passed by value with initializer
#define PYTHON_PROP_I(class_, type, var_name, initializer) \
	type var_name = initializer; \
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

	// Assertions for post_load_validate
#define J_DATA_ASSERT(condition, error_msg)\
	if (!(condition)) { std::string s = this->name; s.append(error_msg); throw jactorio::data::Data_exception(s); }

	
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
		unsigned int internal_id;
		
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


		// Localized names
	protected:
		std::string localized_name_;
		std::string localized_description_;
		
	public:
		J_NODISCARD const std::string& get_localized_name() const {
			return localized_name_;
		}

		virtual void set_localized_name(const std::string& localized_name) {
			this->localized_name_ = localized_name;
		}

		J_NODISCARD const std::string& get_localized_description() const {
			return localized_description_;
		}

		virtual void set_localized_description(const std::string& localized_description) {
			this->localized_description_ = localized_description;
		}

		// ======================================================================
		// Unique data associated with entity
		
		/**
		 * If the prototype has unique data per tile, override the method for deleting it
		 * Deletes unique data for the prototype given through ptr
		 */
		virtual void delete_unique_data(void* ptr) const {
			assert(false);  // Not implemented
		}

		virtual void* copy_unique_data(void* ptr) const {
			assert(false);  // Not implemented
			return nullptr;
		}


		/**
		 * Called after all prototypes are loaded
		 */
		virtual void post_load() {
		}

		/**
		 * Validates properties of the prototype are valid
		 * @exception data::Data_exception If invalid
		 */
		virtual void post_load_validate() const = 0;
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_PROTOTYPE_BASE_H
