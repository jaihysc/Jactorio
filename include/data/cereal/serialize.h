// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_DATA_CEREAL_SERIALIZE_H
#define JACTORIO_DATA_CEREAL_SERIALIZE_H
#pragma once

#include <cstddef>
#include <utility>

// Macros for cereal serialization

#ifdef CEREAL_CEREAL_HPP_
#error serialize.h header should be included before cereal/cereal.hpp
#endif

#ifdef CEREAL_SERIALIE
#error CEREAL_SERIALIZE macro already defined, likely from an external library
#endif

#ifdef CEREAL_LOAD
#error CEREAL_LOAD macro already defined, likely from an external library
#endif

#ifdef CEREAL_SAVE
#error CEREAL_SAVE macro already defined, likely from an external library
#endif

#ifdef CEREAL_LOAD_CONSTRUCT
#error CEREAL_LOAD_CONSTRUCT macro already defined, likey from an external library
#endif


#define CEREAL_SERIALIZE(archiver__)       \
	template <typename TArchive>           \
	void serialize(TArchive& (archiver__))

#define CEREAL_LOAD(archiver__)       \
	template <typename TArchive>      \
	void load(TArchive& (archiver__))

#define CEREAL_SAVE(archiver__)               \
	template <typename TArchive>              \
	void save(TArchive& (archiver__)) const

///
/// \remark For types serialized via smart pointer only
#define CEREAL_LOAD_CONSTRUCT(archiver__, constructor__, data_ty__)                  \
		template <class TArchive>                                                    \
		static void load_and_construct(TArchive& archive,                            \
		                               cereal::construct<data_ty__>& construct)

namespace jactorio::data
{
	///
	/// \brief Size checks arguments to be archived to avoid runtime errors
	template <std::size_t ArchiveSize, typename TArchive, typename ... TArgs>
	void CerealArchive(TArchive& archiver, TArgs&& ... args) {
		static_assert(sizeof...(TArgs) > 0, "At least 1 argument must be provided to archiver");
		static_assert((sizeof(TArgs) + ... + 0) == ArchiveSize, "Provided arguments does not match archive size");

		archiver(std::forward<TArgs>(args) ...);
	}
}

#endif // JACTORIO_DATA_CEREAL_SERIALIZE_H
