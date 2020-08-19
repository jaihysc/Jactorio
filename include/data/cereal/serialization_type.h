// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_DATA_CEREAL_SERIALIZATION_TYPE_H
#define JACTORIO_DATA_CEREAL_SERIALIZATION_TYPE_H
#pragma once

#include <type_traits>

#include "jactorio.h"
#include "core/math.h"
#include "data/prototype_manager.h"
#include "data/cereal/serialize.h"

namespace jactorio::data
{
	///
	/// \brief Manages non owning pointer to prototype
	/// \remark Uses the global active prototype manager for deserializing
	///
	/// Converts pointer to internal id when serializing, from internal id to pointer when deserializing
	template <typename TProto>
	class SerialProtoPtr
	{
		static_assert(std::is_base_of_v<FrameworkBase, TProto>, "TProto must inherit FrameworkBase for internal id");
		static_assert(!std::is_pointer_v<TProto>, "TProto should not be a pointer, remove the indirection");

		using ValueT = std::size_t;
		static constexpr auto kArchiveSize = sizeof(ValueT);

	public:
		SerialProtoPtr() = default;

		SerialProtoPtr(TProto* proto) {  // Intentionally non explicit to allow assignment from pointer directly
			SetProto(proto);
		}

		explicit SerialProtoPtr(TProto& proto) {
			SetProto(&proto);
		}


		TProto* operator->() {
			return GetProto();
		}

		const TProto* operator->() const {
			return GetProto();
		}

		TProto& operator*() {
			return *GetProto();
		}

		const TProto& operator*() const {
			return *GetProto();
		}


		friend bool operator==(const SerialProtoPtr& lhs, const SerialProtoPtr& rhs) {
			return lhs.value_ == rhs.value_;
		}

		friend bool operator!=(const SerialProtoPtr& lhs, const SerialProtoPtr& rhs) {
			return !(lhs == rhs);
		}

		friend void swap(SerialProtoPtr& lhs, SerialProtoPtr& rhs) noexcept {
			using std::swap;
			swap(lhs.value_, rhs.value_);
		}

		J_NODISCARD TProto* Get() noexcept {
			return GetProto();
		}

		J_NODISCARD const TProto* Get() const noexcept {
			return GetProto();
		}


		CEREAL_LOAD(archive) {
			CerealArchive<kArchiveSize>(archive, value_);  // Deserialized as internal id

			if (value_ == 0)  // nullptr
				return;
			
			assert(active_data_manager != nullptr);
			auto* proto_ptr = &active_data_manager->RelocationTableGet<TProto>(  // Converted to prototype*
				core::SafeCast<PrototypeIdT>(value_)
			);
			SetProto(proto_ptr);
		}

		CEREAL_SAVE(archive) {
			ValueT save_val = 0;
			if (value_ != 0)
				save_val = static_cast<ValueT>(GetProto()->internalId);

			CerealArchive<kArchiveSize>(archive, save_val);
		}

	private:

		J_NODISCARD TProto* GetProto() noexcept {
			return reinterpret_cast<TProto*>(value_);
		}

		J_NODISCARD const TProto* GetProto() const noexcept {
			return reinterpret_cast<TProto*>(value_);
		}

		void SetProto(TProto* proto) noexcept {
			value_ = reinterpret_cast<ValueT>(proto);
		}


		/// ptr or internal id
		static_assert(sizeof(ValueT) == sizeof(TProto*));
		ValueT value_ = 0;
	};
}

#endif // JACTORIO_DATA_CEREAL_SERIALIZATION_TYPE_H
