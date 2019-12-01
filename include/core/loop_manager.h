#ifndef CORE_LOOP_MANAGER_H
#define CORE_LOOP_MANAGER_H
/**
 * Keeps the render and logic loops in sync <br>
 * It will wait for the render and logic loops to set a callback to indicate they are initialized
 * The callbacks of the 2 loops will then be called to indicate they should run
 * After completing an iteration of the loop, x_loop_complete should be called to indicate it is done
 */
namespace jactorio::core::loop_manager
{
	using loop_run_callback = void(*)();


	void initialize_loop_manager();
	void terminate_loop_manager();

	/**
	 * @return Whether or not the loop manager is running
	 */
	bool loop_manager_terminated();
	
	// Render loop
	void set_render_refresh_rate(unsigned short refresh_rate);
	unsigned short get_render_refresh_rate();

	/**
	 * Call this after render loop is initiated
	 * @param callback Will be called when the loop should run
	 */
	void render_loop_ready(loop_run_callback callback);
	/**
	 * Call this after completing one cycle of the render loop
	 */
	void render_loop_complete();

	
	// Logic loop
	void set_logic_refresh_rate(unsigned short refresh_rate);
	unsigned short get_logic_refresh_rate();

	/**
	 * Call this after logic loop is initiated
	 * @param callback Will be called when the loop should run
	 */
	void logic_loop_ready(loop_run_callback callback);
	/**
	 * Call this after completing one cycle of the logic loop
	 */
	void logic_loop_complete();
}

#endif // CORE_LOOP_MANAGER_H
