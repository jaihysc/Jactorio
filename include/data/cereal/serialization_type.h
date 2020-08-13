// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_DATA_CEREAL_SERIALIZATION_TYPE_H
#define JACTORIO_DATA_CEREAL_SERIALIZATION_TYPE_H
#pragma once

#include <type_traits>

#include "jactorio.h"
#include "data/prototype_manager.h"
#include "data/cereal/serialize.h"
#include "data/prototype/prototype_base.h"

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
		static_assert(IsValidPrototype<TProto>::value);
	public:

		SerialProtoPtr() = default;

		SerialProtoPtr(TProto* proto)
			: proto_(proto) {
		}

		TProto* operator->() {
			return proto_;
		}

		TProto operator*() {
			return *proto_;
		}

		J_NODISCARD TProto* Get() noexcept {
			return proto_;
		}

		J_NODISCARD const TProto* Get() const noexcept {
			return proto_;
		}


		CEREAL_LOAD(archive) {
			// TODO
		}

		CEREAL_SAVE(archive) {
			archive(proto_);
		}

	private:
		TProto* proto_ = nullptr;
	};
}

#endif // JACTORIO_DATA_CEREAL_SERIALIZATION_TYPE_H
