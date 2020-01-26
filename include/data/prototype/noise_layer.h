#ifndef DATA_PROTOTYPE_NOISE_LAYER_H
#define DATA_PROTOTYPE_NOISE_LAYER_H

#include "data/prototype/prototype_base.h"
#include "data/prototype/tile/tile.h"
#include "data/data_category.h"

#include <vector>

namespace jactorio::data
{
	/**
	 * Note!! MOST values will be in range -1, 1. SOME VALUES will be greater/less than -1, 1
	 */
	class Noise_layer : public Prototype_base
	{
		// The inclusive value where the noise range begins
		std::vector<float> noise_range_tile_ranges_;
		// Size is 1 less than noise_range_tile_ranges
		std::vector<Tile*> noise_range_tiles_;

	public:
		Noise_layer()
			: octave_count(8), frequency(0.25f), persistence(0.5f), normalize_val(false),
			  tile_data_category(data_category::tile) {
			set_start_val(-1.f);
		}

		Noise_layer(const float starting_val, const bool normalize_val)
			: octave_count(8), frequency(0.25f), persistence(0.5f), normalize_val(normalize_val),
			  tile_data_category(data_category::tile) {
			set_start_val(starting_val);
		}

		~Noise_layer() override = default;

		Noise_layer(const Noise_layer& other) = default;

		Noise_layer(Noise_layer&& other) noexcept = default;

		Noise_layer& operator=(const Noise_layer& other) = default;

		Noise_layer& operator=(Noise_layer&& other) noexcept = default;

	public:
		// Perlin noise properties
		// See http://libnoise.sourceforge.net/glossary/ for usage of the following parameters
		PYTHON_PROP_REF(Noise_layer, int, octave_count)
		PYTHON_PROP_REF(Noise_layer, double, frequency)
		PYTHON_PROP_REF(Noise_layer, double, persistence)

	public:

		/**
		 * If true, input values outside of the noise_layer range will be brought to the lowest/highest
		 * If false, input values outside of the noise_layer range returns nullptr
		 */
		PYTHON_PROP_REF(Noise_layer, bool, normalize_val)

		/**
		 * The data category of the tiles which this noise_layer contains
		 */
		PYTHON_PROP_REF(Noise_layer, data_category, tile_data_category)

		
		/**
		 * Retrieves the starting value <br>
		 * Default value is -1
		 * @return
		 */
		[[nodiscard]] float get_start_val() const {
			return noise_range_tile_ranges_[0];
		}

		Noise_layer* set_start_val(const float start_val) {
			// Start_val is first value in the noise_range_tile_ranges vector
			// If already in the vector, modify the first value
			if (noise_range_tile_ranges_.empty())
				noise_range_tile_ranges_.push_back(start_val);
			else
				noise_range_tile_ranges_[0] = start_val;

			return this;
		}

		/**
		 * Returns maximum noise value accepted
		 */
		float get_max_noise_val() {
			// Last item will have highest value
			return noise_range_tile_ranges_[noise_range_tile_ranges_.size() - 1];
		}


		/**
		 * Adds a tile range, the order which this is called affects the order of the tile range <br>
		 * Will not add if start_val is not set
		 * @param end_range Exclusive ending range, unless it is last item
		 * @param tile_ptr Pointer to tile prototype or inheritors
		 */
		Noise_layer* add_tile(float end_range, Tile* tile_ptr);

		/**
		 * Fetches the tile at the current range <br>
		 * normalize_if_val_out_of_range is false, Nullptr if out of range <br>
		 * normalize_if_val_out_of_range is true, Min/max value tile if out of range
		 */
		Tile* get_tile(float val);


		void post_load_validate() const override {
		}
	};
}

#endif // DATA_PROTOTYPE_NOISE_LAYER_H
