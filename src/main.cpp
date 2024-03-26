
#define GL3W_IMPLEMENTATION
#include <gl3w.h>


// internal
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "drawing_system.hpp"
#include "ai_system.hpp"
#include "config.hpp"
#include "movement_system.hpp"

// Entry point
int main()
{
	// Global systems
	WorldSystem world;
	RenderSystem renderer;
	PhysicsSystem physics;
	AISystem ai;

	// Initializing window
	GLFWwindow* window = world.create_window();
	if (!window) {
		// Time to read the error message
		printf("Press any key to exit");
		getchar();
		return EXIT_FAILURE;
	}

	// initialize the main systems
	config.load();
	renderer.init(window);
	world.init(&renderer);
	movementSystem.init();
	physics.init();

	
	// report fps average across this many updates
	const size_t NUM_UPDATES_AVERAGE = 25;
	std::deque<int> frame_counts;
	int frame_update_counter = 20;
	int total_frame_count = 0;
	int curr_fps = 0;

#ifdef __unix__ // linux
	std::string font_filename = "..//data//fonts//Kenney_Pixel.ttf";
#else // windows
	std::string font_filename = "..//..//..//data//fonts//Kenney_Pixel.ttf";
#endif
	unsigned int font_default_size = 60;
	renderer.fontInit(*window, font_filename, font_default_size);
	gl_has_errors();

	// variable timestep loop
	auto t = Clock::now();
	while (!world.is_over()) {
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		world.step(elapsed_ms);
		physics.step(elapsed_ms);
		ai.step(elapsed_ms);
		world.handle_collisions();
		drawings.step(elapsed_ms);
    
		// fps reporting
		if (frame_update_counter++ == 20) {
			frame_update_counter = 0;
			curr_fps = 1000 / elapsed_ms;
			if (debugging.in_debug_mode) printf("actual FPS: %i\n", curr_fps);
			frame_counts.push_back(curr_fps);
			total_frame_count += curr_fps;

			if (frame_counts.size() > NUM_UPDATES_AVERAGE) {
				total_frame_count -= frame_counts.front();
				frame_counts.pop_front();
			}

			curr_fps = total_frame_count / frame_counts.size();

			std::stringstream title_ss;
			title_ss << "Pathfinder - Level: " << world.level + 1 << ", FPS: " << curr_fps;
			glfwSetWindowTitle(window, title_ss.str().c_str());
		}

		renderer.draw();

		glfwSwapBuffers(window);
	}

	return EXIT_SUCCESS;
}
