#ifndef DATA_PROTOTYPE_NOISE_LAYER_H
#define DATA_PROTOTYPE_NOISE_LAYER_H

#include "data/prototype/prototype_base.h"

#include <vector>

#include "data/prototype/tile.h"

namespace jactorio::data
{
	class Noise_layer : public Prototype_base
	{
	    // The inclusive value where the noise range begins
        std::vector<float> noise_range_tile_ranges;
        // Size is 1 less than noise_range_tile_ranges
        std::vector<Tile*> noise_range_tiles;

    public:
	    Noise_layer() {
	        set_start_val(-1);
	    }
	    explicit Noise_layer(int starting_val) {
	        set_start_val(starting_val);
	    }

	    /**
	     * Retrieves the starting value
	     * @throws std::out_of_range if start_val is not set
	     * @return
	     */
        [[nodiscard]] int get_start_val() const {
            return noise_range_tile_ranges[0];
        }

        void set_start_val(const int start_val) {
            // Start_val is first value in the noise_range_tile_ranges vector
            // If already in the vector, modify the first value
            if (noise_range_tile_ranges.empty())
                noise_range_tile_ranges.push_back(start_val);
            else
                noise_range_tile_ranges[0] = start_val;
        }


        /**
         * Adds a tile range, the order which this is called affects the order of the tile range <br>
         * Will not add if start_val is not set
         * @param end_range Exclusive ending range, unless it is last item
         */
        void add_tile(float end_range, Tile* tile_ptr) {
            // Will not add if start_val is not set
            if (noise_range_tile_ranges.empty())
                return;

            noise_range_tile_ranges.push_back(end_range);
            noise_range_tiles.push_back(tile_ptr);
        }

        /**
         * Fetches the tile at the current range <br>
         * Nullptr if out of range
         */
        Tile* get_tile(float val) {
            bool last_value = true;
            for (unsigned int i = noise_range_tile_ranges.size() - 1; i > 0; --i) {
                // Less than
                bool less_than = false;
                if (last_value) {
                    // ending range inclusive only for the last value
                    last_value = false;
                    if (val <= noise_range_tile_ranges[i])
                        less_than = true;
                }
                else {
                    if (val < noise_range_tile_ranges[i])
                        less_than = true;
                }
                if (!less_than)
                    continue;

                // Greater than
                if (val >= noise_range_tile_ranges[i - 1]) {
                    return noise_range_tiles[i - 1];
                }

            }
            // Not found
            return nullptr;
        }
    };
}

#endif // DATA_PROTOTYPE_NOISE_LAYER_H
