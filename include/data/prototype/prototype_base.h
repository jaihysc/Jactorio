// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 11/09/2019

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_PROTOTYPE_BASE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_PROTOTYPE_BASE_H
#pragma once

#include <string>

#include "jactorio.h"

#include "data/data_category.h"
#include "data/data_exception.h"

// Creates a setters for python API primarily, to chain initialization

// Parameter value has suffix a_b_c_d to ensure uniqueness

// Setter passed by reference
#define PYTHON_PROP_REF(class_, type, var_name) \
	type var_name; \
	class_* Set_##var_name(const type& (parameter_value_a_b_c_d)) {\
		this->var_name = parameter_value_a_b_c_d;\
		return this;\
	}

// Setter passed by reference with initializer
#define PYTHON_PROP_REF_I(class_, type, var_name, initializer) \
	type var_name = initializer; \
	class_* Set_##var_name(const type& (parameter_value_a_b_c_d)) {\
		this->var_name = parameter_value_a_b_c_d;\
		return this;\
	}


// Setter passed by value
#define PYTHON_PROP(class_, type, var_name) \
	type var_name; \
	class_* Set_##var_name(type (parameter_value_a_b_c_d)) {\
		this->var_name = parameter_value_a_b_c_d;\
		return this;\
	}

// Setter passed by value with initializer
#define PYTHON_PROP_I(class_, type, var_name, initializer) \
	type var_name = initializer; \
	class_* Set_##var_name(type (parameter_value_a_b_c_d)) {\
		this->var_name = parameter_value_a_b_c_d;\
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
#define J_DATA_ASSERT(condition, format)\
	jactorio::data::DataAssert(condition, "\"%s\", " format, this->name.c_str())

#define J_DATA_ASSERT_F(condition, format, ...)\
	jactorio::data::DataAssert(condition, "\"%s\", " format, this->name.c_str(), __VA_ARGS__)



namespace jactorio::data
{
	class DataManager;

	///
	/// \brief Creates a formatted log message if log level permits
	template <typename ... Args, typename = std::common_type<Args ...>>
	void DataAssert(const bool condition, const char* format, Args&& ... args) {
		constexpr int max_msg_length = 1000;
		
		if (!(condition)) {
			char buffer[max_msg_length + 1];
			snprintf(buffer, max_msg_length, format, args ...);
			throw DataException(buffer);
		}
	}

	///
	/// \brief Abstract base class for all unique data
	struct UniqueDataBase
	{
	protected:
		UniqueDataBase() = default;

	public:
		virtual ~UniqueDataBase() = default;

		UniqueDataBase(const UniqueDataBase& other)                = default;
		UniqueDataBase(UniqueDataBase&& other) noexcept            = default;
		UniqueDataBase& operator=(const UniqueDataBase& other)     = default;
		UniqueDataBase& operator=(UniqueDataBase&& other) noexcept = default;
	};


#define PROTOTYPE_CATEGORY(category_) J_NODISCARD jactorio::data::DataCategory Category() const override { return jactorio::data::DataCategory::category_; }

	class PrototypeBase
	{
	public:
		PrototypeBase()          = default;
		virtual ~PrototypeBase() = default;

		PrototypeBase(const PrototypeBase& other)                = default;
		PrototypeBase(PrototypeBase&& other) noexcept            = default;
		PrototypeBase& operator=(const PrototypeBase& other)     = default;
		PrototypeBase& operator=(PrototypeBase&& other) noexcept = default;

		friend void swap(PrototypeBase& lhs, PrototypeBase& rhs) noexcept {
			using std::swap;
			swap(lhs.category_, rhs.category_);
			swap(lhs.internalId, rhs.internalId);
			swap(lhs.name, rhs.name);
			swap(lhs.order, rhs.order);
			swap(lhs.localizedName_, rhs.localizedName_);
			swap(lhs.localizedDescription_, rhs.localizedDescription_);
		}

	private:
		DataCategory category_ = DataCategory::none;

	public:
		///
		/// \brief Category of this Prototype item
		virtual DataCategory Category() const = 0;

		///
		/// \brief Unique per prototype, unique & auto assigned per new prototype added
		/// 0 indicates invalid id
		unsigned int internalId = 0;


		// ======================================================================
		// Python properties
	public:
		///
		/// \brief Internal name, MUST BE unique per data_category
		///
		PYTHON_PROP_REF(PrototypeBase, std::string, name);

		///
		/// \brief Determines the priority of this prototype used in certain situations
		/// see documentation within inheritors <br>
		/// Automatically assigned incrementally alongside internal_id if not defined <br>
		/// 0 indicates invalid id
		PYTHON_PROP_REF_I(PrototypeBase, unsigned int, order, 0);


		// ======================================================================
		// Localized names
	protected:
		std::string localizedName_;
		std::string localizedDescription_;

	public:
		J_NODISCARD const std::string& GetLocalizedName() const { return localizedName_; }
		virtual void SetLocalizedName(const std::string& localized_name) { this->localizedName_ = localized_name; }

		J_NODISCARD const std::string& GetLocalizedDescription() const { return localizedDescription_; }

		virtual void SetLocalizedDescription(const std::string& localized_description) {
			this->localizedDescription_ = localized_description;
		}


		// ======================================================================
		// Unique data associated with entity

		///
		/// \brief Copies the unique_data associated with a prototype
		virtual UniqueDataBase* CopyUniqueData(UniqueDataBase* ptr) const {
			assert(false);  // Not implemented
			return nullptr;
		}


		// ======================================================================
		// Data Events
		///
		/// \brief Called after all prototypes are loaded prior to validation
		virtual void PostLoad() {
		}

		///
		/// \brief Validates properties of the prototype are valid
		/// \exception data::Data_exception If invalid
		virtual void PostLoadValidate(const DataManager&) const = 0;

		///
		/// \brief Called after the prototype has been validated
		virtual void ValidatedPostLoad() {
		}
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_PROTOTYPE_BASE_H
