// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_DATA_CEREAL_SERIALIZE_H
#define JACTORIO_DATA_CEREAL_SERIALIZE_H
#pragma once

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


#define CEREAL_SERIALIZE(archiver__)\
	template <typename TArchive>\
	void serialize(TArchive& (archiver__))

#define CEREAL_LOAD(archiver__)\
	template <typename TArchive>\
	void load(TArchive& (archiver__))

#define CEREAL_SAVE(archiver__)\
	template <typename TArchive>\
	void save(TArchive& (archiver__))

#endif // JACTORIO_DATA_CEREAL_SERIALIZE_H
