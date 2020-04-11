// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 02/07/2020

#ifndef JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_LAYER_H
#define JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_LAYER_H
#pragma once

#include "data/prototype/prototype_base.h"
#include "game/world/chunk_layer.h"

namespace jactorio::game
{
	struct Multi_tile_data
	{
		/// Number of tiles the sprite should span 
		uint8_t multi_tile_span = 1;
		uint8_t multi_tile_height = 1;
	};

	///
	/// \brief A Chunk_tile has many layers
	/// \remark Commonly instantiated, limit the size of variables
	/// \remark !! Set the multi tile index before using any multi tile methods
	class Chunk_tile_layer : public Chunk_layer
	{
	public:
		Chunk_tile_layer() = default;

		explicit Chunk_tile_layer(const data::Prototype_base* proto)
			: Chunk_layer(proto) {
		}

		~Chunk_tile_layer() {
			clear();
		}

		Chunk_tile_layer(const Chunk_tile_layer& other);
		Chunk_tile_layer(Chunk_tile_layer&& other) noexcept;

		Chunk_tile_layer& operator=(Chunk_tile_layer other) {
			swap(*this, other);
			return *this;
		}

		friend void swap(Chunk_tile_layer& lhs, Chunk_tile_layer& rhs) noexcept {
			using std::swap;
			swap(static_cast<Chunk_layer&>(lhs), static_cast<Chunk_layer&>(rhs));
			swap(lhs.multi_tile_index, rhs.multi_tile_index);
			swap(lhs.multi_tile_data_, rhs.multi_tile_data_);
		}

		///
		/// \brief Resets data on this tile and frees any heap allocated data 
		void clear();


		// ======================================================================
		// Multi tile functionality
		/**
		 * If the layer is multi-tile, eg: 3 x 3
		 * 0 1 2
		 * 3 4 5
		 * 6 7 8
		 */
		uint8_t multi_tile_index = 0;

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
		void* multi_tile_data_ = nullptr;

	public:

		///
		/// \return Whether or not this tile belongs to a multi tile placement
		J_NODISCARD bool is_multi_tile() const {
			if (multi_tile_index != 0)
				assert(multi_tile_data_ != nullptr);  // Multi tiles should never have nullptr for parent_layer

			return multi_tile_data_ != nullptr;
		}

		///
		/// \return Whether or not this is the top left tile of a multi tile placement
		J_NODISCARD bool is_multi_tile_top_left() const {
			if (multi_tile_index != 0)
				assert(multi_tile_data_ != nullptr);  // Multi tiles should never have nullptr for parent_layer

			return multi_tile_data_ != nullptr && multi_tile_index == 0;
		}

		// ======================================================================

		///
		/// \brief Initializes (heap allocates) the properties of the multi tile
		void init_multi_tile_prop(const uint8_t multi_tile_span, const uint8_t multi_tile_height) {
			assert(multi_tile_span > 0);
			assert(multi_tile_height > 0);

			assert(multi_tile_index == 0);  // Only valid for top left multi tiles

			if (multi_tile_data_ == nullptr)
				multi_tile_data_ = new Multi_tile_data{multi_tile_span, multi_tile_height};
			else {
				auto* data = static_cast<Multi_tile_data*>(multi_tile_data_);
				data->multi_tile_span = multi_tile_span;
				data->multi_tile_height = multi_tile_height;
			}
		}


		///
		/// \brief Sets the pointer to the top left multi tile
		void set_multi_tile_parent(Chunk_tile_layer* parent) {
			assert(parent != nullptr);
			assert(multi_tile_index != 0);  // Only valid for tiles other than the top left
			multi_tile_data_ = parent;
		}

		///
		/// \brief Gets the top left multi tile pointer if this is not the top left tile
		J_NODISCARD Chunk_tile_layer* get_multi_tile_parent() const {
			assert(is_multi_tile());
			assert(!is_multi_tile_top_left());

			return static_cast<Chunk_tile_layer*>(multi_tile_data_);
		}

		J_NODISCARD Chunk_tile_layer* get_multi_tile_top_left() {
			assert(is_multi_tile());

			if (is_multi_tile_top_left())
				return this;
			return static_cast<Chunk_tile_layer*>(multi_tile_data_);
		}

		// ======================================================================

		///
		/// \brief Return the multi tile data for the current multi tile placement
		J_NODISCARD Multi_tile_data& get_multi_tile_data() const {
			assert(is_multi_tile());

			if (is_multi_tile_top_left())
				return *static_cast<Multi_tile_data*>(multi_tile_data_);

			return get_multi_tile_parent()->get_multi_tile_data();
		}

		// ======================================================================

		/// \return Number of tiles from top left on X axis
		J_NODISCARD uint8_t get_offset_x() const {
			Multi_tile_data& data = get_multi_tile_data();
			return multi_tile_index % data.multi_tile_span;
		}

		/// \return Number of tiles from top left on Y axis
		J_NODISCARD uint8_t get_offset_y() const {
			Multi_tile_data& data = get_multi_tile_data();
			return multi_tile_index / data.multi_tile_span;
		}
	};

	// NOTE! Multi tiles which are not the top left corner cannot be copied perfectly as they need to point to the top left
	// corner, additional post processing is required to specify the parent
	// Attempting to use methods without specifying the parent will result in assertions failing

	inline Chunk_tile_layer::Chunk_tile_layer(const Chunk_tile_layer& other)
		: Chunk_layer{other},
		  multi_tile_index{other.multi_tile_index} {

		if (other.is_multi_tile_top_left())
			multi_tile_data_ = new Multi_tile_data(*static_cast<Multi_tile_data*>(other.multi_tile_data_));
		else
			multi_tile_data_ = nullptr;
	}

	inline Chunk_tile_layer::Chunk_tile_layer(Chunk_tile_layer&& other) noexcept
		: Chunk_layer{std::move(other)},
		  multi_tile_index{other.multi_tile_index},
		  multi_tile_data_{other.multi_tile_data_} {
		other.multi_tile_data_ = nullptr;
	}

	inline void Chunk_tile_layer::clear() {
		delete unique_data;
		unique_data = nullptr;
		prototype_data = nullptr;

		if (is_multi_tile_top_left())
			delete static_cast<Multi_tile_data*>(multi_tile_data_);
		multi_tile_data_ = nullptr;

		multi_tile_index = 0;
	}
}

#endif //JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_LAYER_H
