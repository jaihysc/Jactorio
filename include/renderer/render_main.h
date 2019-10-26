#ifndef RENDER_MAIN_H
#define RENDER_MAIN_H

namespace jactorio::renderer
{
	void set_recalculate_renderer(unsigned short window_size_x, unsigned short window_size_y);
	
	/**
	 * Sets window refresh rate of renderer
	 * @param refresh_rate Number of times in 1 second to refresh drawn image
	 */
	void set_render_refresh_rate(unsigned short refresh_rate);
	unsigned short get_render_refresh_rate();
	
	void renderer_main();
}

#endif // RENDER_MAIN_H
