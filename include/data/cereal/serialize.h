// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_CEREAL_SERIALIZE_H
#define JACTORIO_INCLUDE_DATA_CEREAL_SERIALIZE_H
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

#ifdef CEREAL_LOAD_EXERN
#error CEREAL_LOAD_EXERN macro already defined, likely from an external library
#endif

#ifdef CEREAL_SAVE
#error CEREAL_SAVE macro already defined, likely from an external library
#endif

#ifdef CEREAL_SAVE_EXTERN
#error CEREAL_SAVE_EXTERN macro already defined, likely from an external library
#endif

#ifdef CEREAL_LOAD_CONSTRUCT
#error CEREAL_LOAD_CONSTRUCT macro already defined, likey from an external library
#endif


#define CEREAL_SERIALIZE(archiver__) \
    template <typename TArchive>     \
    void serialize(TArchive&(archiver__))

#define CEREAL_LOAD(archiver__)  \
    template <typename TArchive> \
    void load(TArchive&(archiver__))

#define CEREAL_LOAD_EXTERN(archiver__, ty__, ty_ref__) \
    template <class Archive>                           \
    void load(Archive&(archiver__), ty__&(ty_ref__))

#define CEREAL_SAVE(archiver__)  \
    template <typename TArchive> \
    void save(TArchive&(archiver__)) const

#define CEREAL_SAVE_EXTERN(archiver__, ty__, ty_ref__) \
    template <class Archive>                           \
    void save(Archive&(archiver__), const ty__&(ty_ref__))

///
/// \remark For types serialized via smart pointer only
#define CEREAL_LOAD_CONSTRUCT(archiver__, constructor__, data_ty__) \
    template <class TArchive>                                       \
    static void load_and_construct(TArchive& archive, cereal::construct<data_ty__>& construct)

namespace jactorio::game
{
    struct GameDataLocal;
    struct GameDataGlobal;
} // namespace jactorio::game

namespace jactorio::data
{
    ///
    /// Size checks arguments to be archived to avoid runtime errors
    template <std::size_t ArchiveSize, typename TArchive, typename... TArgs>
    void CerealArchive(TArchive& archiver, TArgs&&... args) {
        static_assert(sizeof...(TArgs) > 0, "At least 1 argument must be provided to archiver");
        static_assert((sizeof(TArgs) + ... + 0) == ArchiveSize, "Provided arguments does not match archive size");

        archiver(std::forward<TArgs>(args)...);
    }

    void SerializeGameData(const game::GameDataGlobal& game_data);
    ///
    /// \param out_data_global Deserialized into this
    void DeserializeGameData(game::GameDataLocal& data_local, game::GameDataGlobal& out_data_global);
} // namespace jactorio::data

#endif // JACTORIO_INCLUDE_DATA_CEREAL_SERIALIZE_H
