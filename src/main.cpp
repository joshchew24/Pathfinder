
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
#include "particle_system.hpp"

void step_game_logic(WorldSystem& world, PhysicsSystem& physics, AISystem& ai, DrawingSystem& drawings, ParticleSystem& particles, float ms);

// Entry point
int main()
{

	bool value = false;
	bool* mainMenu = &value;

	// Global systems
	WorldSystem world;
	RenderSystem renderer;
	PhysicsSystem physics;
	AISystem ai;
	ParticleSystem particles;

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
	world.init(&renderer, mainMenu);
	movementSystem.init();
	physics.init();
	glfwSetWindowTitle(window, "Pathfinder");

#ifdef __unix__ // linux
	std::string font_filename = "..//data//fonts//Kenney_Pixel.ttf";
#else // windows
	std::string font_filename = "..//..//..//data//fonts//Kenney_Pixel.ttf";
#endif
	unsigned int font_default_size = 60;
	renderer.fontInit(*window, font_filename, font_default_size);
	gl_has_errors();

	// game loop initialization
	auto prevTime = Clock::now();
	auto currentTime = Clock::now();

	float game_logic_accumulator = 0.f;
	float render_accumulator = 0.f;
	float fps_reporting_accumulator = 0.f;

	float elapsed_ms = 0.f;
	float ms_per_tick = 1000.f / config.tick_rate;
	float ms_per_frame = 1000.f / config.target_fps;

	int last_fps_report = 0;
	int num_frames = 0;

	// game loop
	while (!world.is_over()) {
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// Calculating elapsed time in milliseconds from the previous iteration
		currentTime = Clock::now();
		elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(currentTime - prevTime)).count() / 1000;
		prevTime = currentTime;

		if (!*mainMenu) {
			if (config.tick_rate >= 0) {
				game_logic_accumulator += elapsed_ms;
				while (game_logic_accumulator >= ms_per_tick) {
					game_logic_accumulator -= ms_per_tick;
					step_game_logic(world, physics, ai, drawings, particles, ms_per_tick);
				}
			}
			else {
				step_game_logic(world, physics, ai, drawings, particles, elapsed_ms);
			}
		}

		render_accumulator += elapsed_ms;
		if (render_accumulator >= ms_per_frame) {
			num_frames += 1;
			render_accumulator = 0.f;

			renderer.draw();

			// render the fps counter
			std::stringstream fps_display;
			fps_display << "FPS: " << last_fps_report;
			renderer.renderText(fps_display.str().c_str(), 10.f, window_height_px - 20.f, 0.5f, glm::vec3(1.f), glm::mat4(1.f));
			
			glfwSwapBuffers(window);
		}

		fps_reporting_accumulator += elapsed_ms;
		if (fps_reporting_accumulator >= 1000.f) {
			fps_reporting_accumulator = 0.f;
			last_fps_report = num_frames;
			num_frames = 0;
		}

	}

	return EXIT_SUCCESS;
}

void step_game_logic(WorldSystem& world, PhysicsSystem& physics, AISystem& ai, DrawingSystem& drawings, ParticleSystem& particles, float ms) {
	world.step(ms);
	world.handle_collisions(ms);
	physics.step(ms, world.isLineCollisionsOn());
	ai.step(ms);
	drawings.step(ms, world.isLineCollisionsOn());
	particles.step(ms);

}
