// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_LAYER_H
#define JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_LAYER_H
#pragma once

#include "data/cereal/serialization_type.h"
#include "data/cereal/serialize.h"
#include "data/prototype/framework/renderable.h"

#include <cereal/cereal.hpp>

namespace jactorio::game
{
	struct MultiTileData
	{
		using ValueT = uint8_t;

		/// Number of tiles the sprite should span 
		uint8_t span   = 1;
		uint8_t height = 1;

		friend bool operator==(const MultiTileData& lhs, const MultiTileData& rhs) {
			return std::tie(lhs.span, lhs.height) == std::tie(rhs.span, rhs.height);
		}

		friend bool operator!=(const MultiTileData& lhs, const MultiTileData& rhs) {
			return !(lhs == rhs);
		}
	};

	///
	/// \brief A Layers within a ChunkTIle layers
	class ChunkTileLayer
	{
		using MultiTileValueT = MultiTileData::ValueT;

	public:
		ChunkTileLayer() = default;

		explicit ChunkTileLayer(const data::FRenderable* proto)
			: prototypeData(proto) {
		}

		~ChunkTileLayer() = default;

		ChunkTileLayer(const ChunkTileLayer& other);
		ChunkTileLayer(ChunkTileLayer&& other) noexcept;

		ChunkTileLayer& operator=(ChunkTileLayer other) {
			swap(*this, other);
			return *this;
		}


		friend void swap(ChunkTileLayer& lhs, ChunkTileLayer& rhs) noexcept {
			using std::swap;
			swap(lhs.multiTileIndex, rhs.multiTileIndex);
			swap(lhs.prototypeData, rhs.prototypeData);
			swap(lhs.uniqueData_, rhs.uniqueData_);
		}


		///
		/// \brief Resets data on this tile and frees any heap allocated data 
		void Clear() noexcept;


		// Prototype

		///
		/// \tparam T Return type which prototypeData is cast to
		template <typename T = data::FRenderable>
		J_NODISCARD const T* GetPrototypeData() const {
			return static_cast<const T*>(prototypeData.Get());
		}

		// Unique data

		///
		/// \brief Heap allocates unique data
		/// \return Created unique data
		template <typename TData, typename ... Args>
		TData* MakeUniqueData(Args&& ... args) {
			assert(!uniqueData_);  // Trying to create already created uniqueData

			uniqueData_ = std::make_unique<TData>(std::forward<Args>(args) ...);
			return static_cast<TData*>(uniqueData_.get());
		}

		///
		/// \tparam T Return type which uniqueData is cast to
		template <typename T = data::UniqueDataBase>
		J_NODISCARD T* GetUniqueData() noexcept {
			return static_cast<T*>(uniqueData_.get());
		}

		///
		/// \tparam T Return type which uniqueData is cast to
		template <typename T = data::UniqueDataBase>
		J_NODISCARD const T* GetUniqueData() const noexcept {
			return static_cast<const T*>(uniqueData_.get());
		}


		// ======================================================================


		///
		/// \return Whether or not this tile belongs to a multi tile placement
		J_NODISCARD bool IsMultiTile() const {
			if (!HasMultiTileData())
				return false;

			return GetMultiTileData().span != 1 || GetMultiTileData().height != 1;
		}

		///
		/// \return Whether or not this is the top left tile of a multi tile placement
		J_NODISCARD bool IsMultiTileTopLeft() const {
			return IsMultiTile() && multiTileIndex == 0;
		}


		J_NODISCARD bool HasMultiTileData() const {
			return prototypeData != nullptr;
		}

		///
		/// \brief Return the multi tile data for the current multi tile placement
		J_NODISCARD MultiTileData GetMultiTileData() const {
			assert(prototypeData != nullptr);
			return {prototypeData->tileWidth, prototypeData->tileHeight};
		}


		///
		/// \brief Adjusts provided x, y to coordinates of top left tile
		template <typename Tx, typename Ty>
		void AdjustToTopLeft(Tx& x, Ty& y) const {
			if (!IsMultiTile())
				return;

			x -= GetOffsetX();
			y -= GetOffsetY();
		}

		/// \return Number of tiles from top left on X axis
		J_NODISCARD MultiTileValueT GetOffsetX() const {
			const auto& data = GetMultiTileData();
			return multiTileIndex % data.span;
		}

		/// \return Number of tiles from top left on Y axis
		J_NODISCARD MultiTileValueT GetOffsetY() const {
			const auto& data = GetMultiTileData();
			return multiTileIndex / data.span;
		}


		CEREAL_SERIALIZE(archive) {
			archive(prototypeData, multiTileIndex); //, uniqueData_);  // TODO must serialize unique data
		}

		/// A layer may point to a tile prototype to provide additional data (collisions, world gen) 
		data::SerialProtoPtr<const data::FRenderable> prototypeData;


		///	
		/// \brief If the layer is multi-tile, eg: 3 x 2
		/// 0 1 2
		/// 3 4 5
		MultiTileValueT multiTileIndex = 0;

	private:
		/// Data for the prototype which is unique per tile and layer
		std::unique_ptr<data::UniqueDataBase> uniqueData_;
	};

	inline ChunkTileLayer::ChunkTileLayer(const ChunkTileLayer& other)
		: prototypeData(other.prototypeData),
		  multiTileIndex{other.multiTileIndex} {

		// Use prototype defined method for copying uniqueData_ if other has data to copy
		if (other.uniqueData_ != nullptr) {
			assert(other.prototypeData.Get() != nullptr);  // No prototype_data_ available for copying unique_data_
			uniqueData_ = other.prototypeData->CopyUniqueData(other.uniqueData_.get());
		}
	}

	inline ChunkTileLayer::ChunkTileLayer(ChunkTileLayer&& other) noexcept
		: prototypeData(other.prototypeData),
		  multiTileIndex{other.multiTileIndex},
		  uniqueData_(std::move(other.uniqueData_)) {
	}

	inline void ChunkTileLayer::Clear() noexcept {
		uniqueData_.reset();
		prototypeData  = nullptr;
		multiTileIndex = 0;
	}
}

#endif //JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_LAYER_H
