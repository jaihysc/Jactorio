// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_LAYER_H
#define JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_LAYER_H
#pragma once

#include "data/prototype/framework/framework_base.h"
#include "game/world/chunk_layer.h"

namespace jactorio::game
{
	struct MultiTileData
	{
		/// Number of tiles the sprite should span 
		uint8_t multiTileSpan   = 1;
		uint8_t multiTileHeight = 1;
	};

	///
	/// \brief A Chunk_tile has many layers
	/// \remark Commonly instantiated, limit the size of variables
	/// \remark !! Set the multi tile index before using any multi tile methods
	class ChunkTileLayer : public ChunkLayer
	{
	public:
		ChunkTileLayer() = default;

		explicit ChunkTileLayer(const data::FrameworkBase* proto)
			: ChunkLayer(proto) {
		}

		~ChunkTileLayer() {
			Clear();
		}

		ChunkTileLayer(const ChunkTileLayer& other);
		ChunkTileLayer(ChunkTileLayer&& other) noexcept;

		ChunkTileLayer& operator=(ChunkTileLayer other) {
			swap(*this, other);
			return *this;
		}

		friend void swap(ChunkTileLayer& lhs, ChunkTileLayer& rhs) noexcept {
			using std::swap;
			swap(static_cast<ChunkLayer&>(lhs), static_cast<ChunkLayer&>(rhs));
			swap(lhs.multiTileIndex, rhs.multiTileIndex);
			swap(lhs.multiTileData_, rhs.multiTileData_);
		}

		///
		/// \brief Resets data on this tile and frees any heap allocated data 
		void Clear();


		// ======================================================================
		// Multi tile functionality
		///	
		/// \brief If the layer is multi-tile, eg: 3 x 3
		/// 0 1 2
		/// 3 4 5
		/// 6 7 8
		uint8_t multiTileIndex = 0;

	private:
		/*
		 * Multi tiles:
		 *
		 * 1. Unique data is stored in the top left tile, indicated by:
		 *		1. multi_tile_index = 0
		 *		2. parent_layer != nullptr
		 *
		 * 2. Non-top left tiles holds a pointer to the top left tile, indicated by:
		 *		1. multi_tile_index != 0
		 *		parent_layer should always != nullptr
		 */


		/*
		 * For multi tile prototypes, this serves 2 purposes
		 *
		 * 1. Hold a pointer to the top left tile
		 * 2. Holds a pointer to the properties of the multi tile if this is the top left tile
		 */
		void* multiTileData_ = nullptr;

	public:

		///
		/// \return Whether or not this tile belongs to a multi tile placement
		J_NODISCARD bool IsMultiTile() const {
			if (multiTileIndex != 0)
				assert(multiTileData_ != nullptr);  // Multi tiles should never have nullptr for parent_layer

			return multiTileData_ != nullptr;
		}

		///
		/// \return Whether or not this is the top left tile of a multi tile placement
		J_NODISCARD bool IsMultiTileTopLeft() const {
			if (multiTileIndex != 0)
				assert(multiTileData_ != nullptr);  // Multi tiles should never have nullptr for parent_layer

			return multiTileData_ != nullptr && multiTileIndex == 0;
		}

		// ======================================================================

		///
		/// \brief Initializes (heap allocates) the properties of the multi tile
		void InitMultiTileProp(const uint8_t multi_tile_span, const uint8_t multi_tile_height) {
			assert(multi_tile_span > 0);
			assert(multi_tile_height > 0);

			assert(multiTileIndex == 0);  // Only valid for top left multi tiles

			if (multiTileData_ == nullptr)
				multiTileData_ = new MultiTileData{multi_tile_span, multi_tile_height};
			else {
				auto* data            = static_cast<MultiTileData*>(multiTileData_);
				data->multiTileSpan   = multi_tile_span;
				data->multiTileHeight = multi_tile_height;
			}
		}


		///
		/// \brief Sets the pointer to the top left multi tile
		void SetMultiTileParent(ChunkTileLayer* parent) {
			assert(parent != nullptr);
			assert(multiTileIndex != 0);  // Only valid for tiles other than the top left
			multiTileData_ = parent;
		}

		///
		/// \brief Gets the top left multi tile pointer if this is not the top left tile
		J_NODISCARD ChunkTileLayer* GetMultiTileParent() const {
			assert(IsMultiTile());
			assert(!IsMultiTileTopLeft());

			return static_cast<ChunkTileLayer*>(multiTileData_);
		}

		///
		/// \brief Gets top left layer if is multi tile, otherwise itself if not a multi tile
		J_NODISCARD ChunkTileLayer& GetMultiTileTopLeft() {
			if (!IsMultiTile() || IsMultiTileTopLeft())
				return *this;

			auto* val = static_cast<ChunkTileLayer*>(multiTileData_);
			assert(val);
			return *val;
		}

		// ======================================================================

		///
		/// \brief Return the multi tile data for the current multi tile placement
		J_NODISCARD MultiTileData& GetMultiTileData() const {
			assert(IsMultiTile());

			if (IsMultiTileTopLeft())
				return *static_cast<MultiTileData*>(multiTileData_);

			return GetMultiTileParent()->GetMultiTileData();
		}

		// ======================================================================

		///
		/// \brief Adjusts provided x, y to coordinates of top left tile
		template <typename Tx, typename Ty>
		void AdjustToTopLeft(Tx& x, Ty& y) {
			if (!IsMultiTile())
				return;

			x -= GetOffsetX();
			y -= GetOffsetY();
		}

		/// \return Number of tiles from top left on X axis
		J_NODISCARD uint8_t GetOffsetX() const {
			MultiTileData& data = GetMultiTileData();
			return multiTileIndex % data.multiTileSpan;
		}

		/// \return Number of tiles from top left on Y axis
		J_NODISCARD uint8_t GetOffsetY() const {
			MultiTileData& data = GetMultiTileData();
			return multiTileIndex / data.multiTileSpan;
		}
	};

	// NOTE! Multi tiles which are not the top left corner cannot be copied perfectly as they need to point to the top left
	// corner, additional post processing is required to specify the parent
	// Attempting to use methods without specifying the parent will result in assertions failing

	inline ChunkTileLayer::ChunkTileLayer(const ChunkTileLayer& other)
		: ChunkLayer{other},
		  multiTileIndex{other.multiTileIndex} {

		if (other.IsMultiTileTopLeft())
			multiTileData_ = new MultiTileData(*static_cast<MultiTileData*>(other.multiTileData_));
		else
			multiTileData_ = nullptr;
	}

	inline ChunkTileLayer::ChunkTileLayer(ChunkTileLayer&& other) noexcept
		: ChunkLayer{std::move(other)},
		  multiTileIndex{other.multiTileIndex},
		  multiTileData_{other.multiTileData_} {
		other.multiTileData_ = nullptr;
	}

	inline void ChunkTileLayer::Clear() {
		delete uniqueData_;
		uniqueData_   = nullptr;
		prototypeData = nullptr;

		if (IsMultiTileTopLeft())
			delete static_cast<MultiTileData*>(multiTileData_);
		multiTileData_ = nullptr;

		multiTileIndex = 0;
	}
}

#endif //JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_LAYER_H
