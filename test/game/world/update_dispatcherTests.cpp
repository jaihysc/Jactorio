// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/world/update_dispatcher.h"
#include "game/world/world_data.h"

namespace jactorio::game
{
	class UpdateDispatcherTest : public testing::Test
	{
		class MockUpdateListener final : public data::IUpdateListener
		{
		public:
			mutable WorldCoord emit;
			mutable WorldCoord receive;
			mutable int calls = 0;

			mutable data::UpdateType type = data::UpdateType::remove;

			void OnTileUpdate(WorldData&,
			                  const WorldCoord& emit_coords,
			                  const WorldCoord& receive_coords,
			                  const data::UpdateType type) const override {
				emit    = emit_coords;
				receive = receive_coords;

				calls++;
				this->type = type;
			}
		};

	protected:
		WorldData worldData_;
		UpdateDispatcher& dispatcher_ = worldData_.updateDispatcher;

		MockUpdateListener mock_{};
	};

	TEST_F(UpdateDispatcherTest, Register) {
		dispatcher_.Register(2, 3,
		                     5, 6, mock_);

		dispatcher_.Dispatch(worldData_, 3, 7, data::UpdateType::place);
		EXPECT_EQ(mock_.emit.x, 0);
		EXPECT_EQ(mock_.emit.y, 0);

		EXPECT_EQ(mock_.receive.x, 0);
		EXPECT_EQ(mock_.receive.y, 0);


		dispatcher_.Dispatch(worldData_, 5, 6, data::UpdateType::remove);

		EXPECT_EQ(mock_.emit.x, 5);
		EXPECT_EQ(mock_.emit.y, 6);

		EXPECT_EQ(mock_.receive.x, 2);
		EXPECT_EQ(mock_.receive.y, 3);

		EXPECT_EQ(mock_.type, data::UpdateType::remove);
	}

	TEST_F(UpdateDispatcherTest, Unregister) {
		const auto entry = dispatcher_.Register(2, 3,
		                                        5, 6, mock_);

		dispatcher_.Register(4, 7,
		                     5, 6, mock_);

		dispatcher_.Unregister(entry);

		// 1 registered, 1 unregistered
		dispatcher_.Dispatch(worldData_, 5, 6, data::UpdateType::place);
		EXPECT_EQ(mock_.emit.x, 5);
		EXPECT_EQ(mock_.emit.y, 6);

		EXPECT_EQ(mock_.receive.x, 4);
		EXPECT_EQ(mock_.receive.y, 7);
		EXPECT_EQ(mock_.calls, 1);
	}

	TEST_F(UpdateDispatcherTest, RegisterNonExistent) {
		dispatcher_.Dispatch(worldData_, 3, 7, data::UpdateType::place);
		dispatcher_.Dispatch(worldData_, 5, 6, data::UpdateType::remove);

		EXPECT_EQ(dispatcher_.GetDebugInfo().storedEntries.size(), 0);
	}

	TEST_F(UpdateDispatcherTest, UnregisterEmptyErase) {
		const auto entry = dispatcher_.Register(2, 3,
		                                        5, 6, mock_);
		dispatcher_.Unregister(entry);

		EXPECT_EQ(dispatcher_.GetDebugInfo().storedEntries.size(), 0);
	}
}
