#include "game/world/world_manager.h"

#include <unordered_map>
#include <future>
#include <mutex>

#include "core/data_type/unordered_map.h"

using world_chunks_key = unsigned long long;

// world_chunks_key correlate to a chunk
std::unordered_map<std::tuple<int, int>, jactorio::game::Chunk*,
                   jactorio::core::hash<std::tuple<int, int>>> world_chunks;

std::mutex m_world_chunks;
// /**
//  * 3 possible states for unordered map write lock
//  * 0 - Not locked
//  * 1 - Awaiting lock
//  * 2 - Locked
//  */
// std::atomic<unsigned short> mp_world_chunks_write_lock_status = 0;
// std::atomic<int> mp_world_chunks_read_thread_count = 0;
// std::condition_variable mp_world_chunks_cv;  // Used to notify lock gained

jactorio::game::Chunk* jactorio::game::world_manager::add_chunk(Chunk* chunk) {
	const auto position = chunk->get_position();
	const auto coords = std::tuple<int, int>{ position.first, position.second};

	std::lock_guard<std::mutex> guard(m_world_chunks);

	// A chunk already exist at this position?
	if (world_chunks.find(coords) != world_chunks.end()) {
		delete world_chunks[coords];
	}
	
	world_chunks[coords] = chunk;

	// Return pointer to allocated chunk
	return chunk;
}

inline jactorio::game::Chunk* p_get_chunk(int x, int y) {
	const auto key = std::tuple<int, int>{ x, y };

	if (world_chunks.find(key) == world_chunks.end())
		return nullptr;

	return world_chunks[key];
}

jactorio::game::Chunk* jactorio::game::world_manager::get_chunk(const int x, const int y) {
	std::lock_guard<std::mutex> guard(m_world_chunks);
	return p_get_chunk(x, y);

	// Unordered_map must be locked to prevent writing to while reading from it
	// Once one read thread locks the mutex, subsequent read threads do not need to lock

	// auto unlock_guard = core::Resource_guard<void>([]() {
	// 	// Unlock if this is the final read thread and it is locked
	// 	--mp_world_chunks_read_thread_count;
	// 	if (mp_world_chunks_read_thread_count == 0 && mp_world_chunks_write_lock_status == 2) {
	// 		mp_world_chunks_write_lock_status = 0;
	// 		m_world_chunks.unlock();
	// 	}
	// 	else {
	// 		mp_world_chunks_cv.notify_one();
	// 	}
	// });
	// ++mp_world_chunks_read_thread_count;
	//
	// // Has not been locked yet
	// unsigned short zero = 0;
	// const unsigned short one = 1;
	// if (mp_world_chunks_write_lock_status.compare_exchange_weak(zero, one)) {
	// 	m_world_chunks.lock();
	// 	mp_world_chunks_write_lock_status = 2;  // Lock acquired
	// 	
	// 	return p_get_chunk(x, y);
	// }
	//
	// // Another read thread is already waiting for a lock
	// if (mp_world_chunks_write_lock_status == 1) {
	// 	// Wait until lock acquired
	// 	std::mutex m;
	// 	std::unique_lock<std::mutex> lk(m);
	// 	mp_world_chunks_cv.wait(lk, [&](){ return p_get_chunk(x, y); });
	// }
	//
	// // Lock already acquired
	// return p_get_chunk(x, y);
}

jactorio::game::Chunk_tile* jactorio::game::world_manager::get_tile_world_coords(int world_x,
                                                                                 int world_y) {

	// The negative chunks start at -1, unlike positive chunks at 0
	// Thus add 1 to become 0 so the calculations can be performed
	bool negative_x = false;
	bool negative_y = false;

	float chunk_index_x = 0;
	float chunk_index_y = 0;

	if (world_x < 0) {
		negative_x = true;
		chunk_index_x -= 1;
		world_x += 1;
	}
	if (world_y < 0) {
		negative_y = true;
		chunk_index_y -= 1;
		world_y += 1;
	}

	chunk_index_x += static_cast<float>(world_x) / 32;
	chunk_index_y += static_cast<float>(world_y) / 32;


	auto* chunk = get_chunk(static_cast<int>(chunk_index_x), static_cast<int>(chunk_index_y));

	if (chunk != nullptr) {
		int tile_index_x = static_cast<int>(world_x) % 32;
		int tile_index_y = static_cast<int>(world_y) % 32;

		// Chunk is 32 tiles
		if (negative_x) {
			tile_index_x = 31 - tile_index_x * -1;
		}
		if (negative_y) {
			tile_index_y = 31 - tile_index_y * -1;
		}

		return &chunk->tiles_ptr()[32 * tile_index_y + tile_index_x];
	}

	return nullptr;
}

void jactorio::game::world_manager::clear_chunk_data() {
	std::lock_guard<std::mutex> guard(m_world_chunks);

	// The chunk data itself needs to be deleted
	for (auto& world_chunk : world_chunks) {
		delete world_chunk.second;
	}
	world_chunks.clear();
}
