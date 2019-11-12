#ifndef CORE_LOOP_MANAGER_H
#define CORE_LOOP_MANAGER_H
/**
 * Keeps the render and logic loops in sync <br>
 * It will wait for the render and logic loops to set a callback to indicate they are initialized
 * The callbacks of the 2 loops will then be called to indicate they should run
 * After completing an iteration of the loop, x_loop_complete should be called to indicate it is done
 * TODO add a error if loop has not called complete on next iteration
 */
namespace jactorio::core::loop_manager
{
	using loop_run_callback = void(*)();


	void initialize_loop_manager();
	void terminate_loop_manager();
	
	// Render loop
	void set_render_refresh_rate(unsigned short refresh_rate);
	unsigned short get_render_refresh_rate();
	
	void render_loop_ready(loop_run_callback callback);
	void render_loop_complete();
	
	// Logic loop
	void set_logic_refresh_rate(unsigned short refresh_rate);
	unsigned short get_logic_refresh_rate();
	
	void logic_loop_ready(loop_run_callback callback);
	void logic_loop_complete();
}

#endif // CORE_LOOP_MANAGER_H
