#include <gtest/gtest.h>

#include "core/loop_manager.h"

TEST(render_main, render_refresh_rate_get_set) {
	// Defaults to 60
	EXPECT_EQ(jactorio::core::loop_manager::get_render_refresh_rate(), 60);


	jactorio::core::loop_manager::set_render_refresh_rate(123);
	EXPECT_EQ(jactorio::core::loop_manager::get_render_refresh_rate(), 123);
}

TEST(render_main, logic_refresh_rate_get_set) {
	// Defaults to 60
	EXPECT_EQ(jactorio::core::loop_manager::get_logic_refresh_rate(), 60);


	jactorio::core::loop_manager::set_logic_refresh_rate(321);
	EXPECT_EQ(jactorio::core::loop_manager::get_logic_refresh_rate(), 321);
}