
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

#ifdef __unix__ // linux
	std::string font_filename = "..//data//fonts//Kenney_Pixel.ttf";
#else // windows
	std::string font_filename = "..//..//..//data//fonts//Kenney_Pixel.ttf";
#endif
	unsigned int font_default_size = 60;
	renderer.fontInit(*window, font_filename, font_default_size);
	gl_has_errors();

	// variable timestep loop
	auto prevTime = Clock::now();
	auto currentTime = Clock::now();
	float game_logic_accumulator = 0.f;
	float render_accumulator = 0.f;
	float fps_reporting_accumulator = 0.f;
	float elapsed_ms = 0.f;
	float ms_per_tick = 1000.f / config.tick_rate;
	float ms_per_frame = 1000.f / config.target_fps;

	int num_frames = 0;

	while (!world.is_over()) {
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		currentTime = Clock::now();
		elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(currentTime - prevTime)).count() / 1000;
		prevTime = currentTime;

		game_logic_accumulator += elapsed_ms;
		while (game_logic_accumulator >= ms_per_tick) {
			game_logic_accumulator -= ms_per_tick;

			world.step(ms_per_tick);
			world.handle_collisions(ms_per_tick);
			physics.step(ms_per_tick);
			ai.step(ms_per_tick);
			drawings.step(ms_per_tick);
		}

		render_accumulator += elapsed_ms;
		if (render_accumulator >= ms_per_frame) {
			renderer.draw();
			glfwSwapBuffers(window);
			num_frames += 1;
			render_accumulator = 0.f;
		}

		fps_reporting_accumulator += elapsed_ms;
		if (fps_reporting_accumulator >= 1000.f) {
			std::stringstream title_ss;
			title_ss << "Pathfinder - Level: " << world.level + 1 << ", FPS: " << num_frames;
			glfwSetWindowTitle(window, title_ss.str().c_str());
			fps_reporting_accumulator = 0.f;
			num_frames = 0;
		}

	}

	return EXIT_SUCCESS;
}
