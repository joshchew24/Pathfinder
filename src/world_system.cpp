// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>

#include "physics_system.hpp"
#include "movement_system.hpp"
#include <ai_system.hpp>

// Game configuration
const size_t MAX_BOULDERS = 5;
const size_t MAX_BUG = 5;
const size_t BOULDER_DELAY_MS = 2000 * 3;
const size_t BUG_DELAY_MS = 5000 * 3;
const float FRICTION = 5.f;

// Create the bug world
WorldSystem::WorldSystem()
	: next_boulder_spawn(0.f)
	, next_bug_spawn(0.f) {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	if (chicken_dead_sound != nullptr)
		Mix_FreeChunk(chicken_dead_sound);
	if (chicken_eat_sound != nullptr)
		Mix_FreeChunk(chicken_eat_sound);
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char *desc) {
		fprintf(stderr, "%d: %s", error, desc);
	}
}


// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW");
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(window_width_px, window_height_px, "Pathfinder", nullptr, nullptr);
	if (window == nullptr) {
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);

	// Set cursor mode to hidden
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	chicken_dead_sound = Mix_LoadWAV(audio_path("chicken_dead.wav").c_str());
	chicken_eat_sound = Mix_LoadWAV(audio_path("chicken_eat.wav").c_str());

	if (background_music == nullptr || chicken_dead_sound == nullptr || chicken_eat_sound == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("music.wav").c_str(),
			audio_path("chicken_dead.wav").c_str(),
			audio_path("chicken_eat.wav").c_str());
		return nullptr;
	}

	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg) {
	this->renderer = renderer_arg;
	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");

	//init levels
	WorldSystem::level = 0;
	LevelManager lm;
	lm.initLevel();
	lm.printLevelsInfo();
	this->levelManager = lm;
	// Set all states to default
    restart_game();
}

std::pair<float, float> advancedAIlerp(float x0, float y0, float x1, float y1, float t) {
	//printf("x0:%f\n", x0);
	//printf("x1:%f\n", x1);
	//printf("y0:%f\n", y0);
	//printf("y1:%f\n", y1);
	float x = x0 + t * (x1 - x0);
	float y = y0 + t * (y1 - y0);
	return { x, y };
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
	    registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto& motions_registry = registry.motions;
	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motions_registry.components.size()-1; i>=0; --i) {
	    Motion& motion = motions_registry.components[i];
		if (motion.position.x + abs(motion.scale.x) < 0.f) {
			if(!registry.players.has(motions_registry.entities[i])) // don't remove the player
				registry.remove_all_components_of(motions_registry.entities[i]);
		}
	}

	Motion& pmotion = registry.motions.get(player);
	//vec2 pPosition = pmotion.position;

	// if entity is player and below window screen
	if (pmotion.position.y - abs(pmotion.scale.y) / 2 > window_height_px) {
		if (!registry.deathTimers.has(player)) {
			registry.deathTimers.emplace(player);
			Mix_PlayChannel(-1, chicken_dead_sound, 0);
		}
	}

	next_boulder_spawn -= elapsed_ms_since_last_update * current_speed * 2;
	if (level >= 1 && registry.deadlys.components.size() <= MAX_BOULDERS && next_boulder_spawn < 0.f) {
		// Reset timer
		next_boulder_spawn = (BOULDER_DELAY_MS / 2) + uniform_dist(rng) * (BOULDER_DELAY_MS / 2);
		createBoulder(renderer, vec2(50.f + uniform_dist(rng) * (window_width_px - 100.f), -100.f));
	}


	if(!registry.deathTimers.has(player) && level >= 2)
	{
		FrameCount += elapsed_ms_since_last_update;
		if (FrameCount / msPerFrame >= FrameInterval) {
			aiSystem.updateGrid(levelManager.levels[level].walls);
			//System.printGrid();
			Motion& eMotion = registry.motions.get(advancedBoulder);
			Motion& pMotion = registry.motions.get(player);
			bestPath = aiSystem.bestPath(eMotion, pMotion);
			currentNode = 0;
			FrameCount = 0;
		}

		Motion& eMotion = registry.motions.get(advancedBoulder);

		//std::cout << "Path found: ";
		//for (const auto& p : bestPath) {
		//	std::cout << "(" << p.first << ", " << p.second << ") ";

		//}
		//std::cout << std::endl;

		if (bestPath.size() != 0 && currentNode < bestPath.size() - 1) {
			float x0 = eMotion.position.x;
			float y0 = eMotion.position.y;
			float x1 = (bestPath[currentNode + 1].first) * gridSize;
			float y1 = (bestPath[currentNode + 1].second + 1) * gridSize;

			//printf("x0:%f\n", x0);
			//printf("x1:%f\n", x1);

			//printf("y0:%f\n", y0);
			//printf("y1:%f\n", y1);

			if (x0 > x1) {
				x1 = (bestPath[currentNode + 1].first) * gridSize;
			}

			if (y0 > y1) {
				y1 = (bestPath[currentNode + 1].second) * gridSize;
			}

			float distance = std::sqrt(std::pow(x1 - x0, 2) + std::pow(y1 - y0, 2));
			//printf("distance:%f\n", distance);
			if (distance < 1) {
				currentNode++;
			}
			else {
				auto interpolatedPoint = advancedAIlerp(x0, y0, x1, y1, 0.05);
				eMotion.position.x = interpolatedPoint.first;
				eMotion.position.y = interpolatedPoint.second;
			}
		}
	}

	//advanced AI
	//for (int x = 0; x <= window_width_px; x += gridSize) {
	//	createLine({ x, window_height_px / 2 }, { 3, window_height_px });
	//}
	//for (int y = 0; y <= window_height_px; y += gridSize) {
	//	createLine({ window_width_px / 2, y }, { window_width_px, 3 });
	//}

	// Processing the chicken state
	assert(registry.screenStates.components.size() <= 1);
    ScreenState &screen = registry.screenStates.components[0];

    float min_counter_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities) {
		// progress timer
		DeathTimer& counter = registry.deathTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if(counter.counter_ms < min_counter_ms){
		    min_counter_ms = counter.counter_ms;
		}

		// restart the game once the death timer expired
		if (counter.counter_ms < 0) {
			registry.deathTimers.remove(entity);
			screen.darken_screen_factor = 0;
            restart_game();
			return true;
		}
	}
	// reduce window brightness if any of the present chickens is dying
	screen.darken_screen_factor = 1 - min_counter_ms / 3000;

	movementSystem.handle_inputs();
	return true;
}

void WorldSystem::createLevel() {
	Level currentLevel = this->levelManager.levels[WorldSystem::level];
	for (int i = 0; i < currentLevel.walls.size(); ++i) {
		initWall w = currentLevel.walls[i];
		createWall(renderer, {w.x, w.y}, {w.xSize, w.ySize});
		int platformHeight = abs(w.y - window_height_px) + w.ySize / 2 + 2;
		createPlatform(renderer, {w.x, window_height_px - platformHeight}, {w.xSize - 10, 10.f});
	}
	createCheckpoint(renderer, { currentLevel.checkpoint.first, currentLevel.checkpoint.second });
	createEndpoint(renderer, { currentLevel.endPoint.first, currentLevel.endPoint.second });
	player = createOliver(renderer, { currentLevel.playerPos.first, currentLevel.playerPos.second });
	registry.colors.insert(player, { 1, 0.8f, 0.8f });
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset the game speed
	current_speed = 1.f;

	movementSystem.reset();

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all bug, eagles, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
	    registry.remove_all_components_of(registry.motions.entities.back());

	// Remove pencil
	while (registry.pencil.entities.size() > 0)
		registry.remove_all_components_of(registry.pencil.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();
	
	createBackground(renderer);

	//platform
	createLevel();
	
	// Create pencil
	pencil = createPencil(renderer, { window_width_px / 2, window_height_px / 2 }, { 50.f, 50.f });

	// Create test paint can

	// Center cursor to pencil location
	glfwSetCursorPos(window, window_width_px / 2 - 25.f, window_height_px / 2 + 25.f);

	if (level >= 2) {
		advancedBoulder = createChaseBoulder(renderer, { window_width_px / 2, 100 });
		createPaintCan(renderer, { window_width_px - 300, window_height_px / 2 }, { 55.f, 55.f });
	}

}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions;
	Motion& pMotion = registry.motions.get(player);
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other;

		// For now, we are only interested in collisions that involve the chicken
		if (registry.players.has(entity)) {
			//Player& player = registry.players.get(entity);

			// Checking Player - Deadly collisions
			if (registry.deadlys.has(entity_other)) {
				// initiate death unless already dying
				if (!registry.deathTimers.has(entity)) {
					// Scream, reset timer, and make the chicken sink
					registry.deathTimers.emplace(entity);
					Mix_PlayChannel(-1, chicken_dead_sound, 0);

					// !!! TODO A1: change the chicken orientation and color on death
				}
			}
			// Checking Player - Eatable collisions
			else if (registry.eatables.has(entity_other)) {
				if (!registry.deathTimers.has(entity)) {
					// chew, count points, and set the LightUp timer
					registry.remove_all_components_of(entity_other);
					Mix_PlayChannel(-1, chicken_eat_sound, 0);

					// !!! TODO A1: create a new struct called LightUp in components.hpp and add an instance to the chicken entity by modifying the ECS registry
				}
			}
			else if (registry.walls.has(entity_other)) {
				Motion& pMotion = registry.motions.get(entity);
				Motion& wMotion = registry.motions.get(entity_other);
				float leftMax = wMotion.position.x - wMotion.scale.x / 2 + 50;
				float rightMax = wMotion.position.x + wMotion.scale.x / 2 - 50;
				if (pMotion.position.x <= leftMax) {
					pMotion.position.x = leftMax - 70;
				}
				else {
					pMotion.position.x = rightMax + 70;
				}
			}

			// Checking Player - Checkpoint collisions
			else if (registry.checkpoints.has(entity_other)) {
				save_checkpoint();
			}

			else if (registry.levelEnds.has(entity_other)) {
				next_level();
			}
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

void WorldSystem::next_level() {
	if (level == maxLevel) {
		level = 0;
		restart_game();
	}
	else {
		level++;
		restart_game();
	}
}

void WorldSystem::save_checkpoint() {
	Motion m = registry.motions.get(player);
	json j;
	// Save position, but not velocity; no need to preserve momentum from time of save
	j["position"]["x"] = m.position[0];
	j["position"]["y"] = m.position[1];
	j["scale"]["x"] = m.scale[0];
	j["scale"]["y"] = m.scale[1];
	j["gravity"] = m.gravityScale;
	j["level"] = level;

	std::ofstream o("../save.json");
	o << j.dump() << std::endl;
}

void WorldSystem::load_checkpoint() {
	std::ifstream i("../save.json");
	// Ensure file exists
	if (!i.good())
		return;
	
	json j = json::parse(i);

	int currentLevel = j["level"];
	if (currentLevel == level) {
		// reset game to default then reposition player
		restart_game();
		Motion& m = registry.motions.get(player);
		m.position[0] = j["position"]["x"];
		m.position[1] = j["position"]["y"];
		m.scale[0] = j["scale"]["x"];
		m.scale[1] = j["scale"]["y"];
		m.gravityScale = j["gravity"];
	}

}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
	//close on escape
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose(window, 1);
	}

	// player movement
	if ((key == movementSystem.RIGHT_KEY || key == movementSystem.LEFT_KEY) && !registry.deathTimers.has(player)) {
		RenderRequest& renderRequest = registry.renderRequests.get(player);
		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			movementSystem.press(key);
			if (key == movementSystem.LEFT_KEY) {
				if (currentRunningTexture == (int)TEXTURE_ASSET_ID::RUN4) {
					currentRunningTexture = (int)TEXTURE_ASSET_ID::OLIVER - 1;
				}
				currentRunningTexture++;
				renderRequest.used_texture = static_cast<TEXTURE_ASSET_ID>(currentRunningTexture);
			}
			else if (key == movementSystem.RIGHT_KEY) {
				if (currentRunningTexture == (int)TEXTURE_ASSET_ID::RUN4) {
					currentRunningTexture = (int)TEXTURE_ASSET_ID::OLIVER - 1;
				}
				currentRunningTexture++;
				renderRequest.used_texture = static_cast<TEXTURE_ASSET_ID>(currentRunningTexture);
			}
		}
		else if (action == GLFW_RELEASE) {
			movementSystem.release(key);
			renderRequest.used_texture = TEXTURE_ASSET_ID::OLIVER;
		}
	}

	// player jump
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		auto& motions = registry.motions;
		Motion& motion = motions.get(player);
		if (motion.grounded && !registry.deathTimers.has(player)) {
			motion.velocity.y = -600.f;
			motion.grounded = false;
		}
	}


	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);

        restart_game();
	}

	// Loading game
	if (action == GLFW_RELEASE && key == GLFW_KEY_L) {
		load_checkpoint();
	}

	// Debugging
	if (key == GLFW_KEY_I) {
		if (action == GLFW_PRESS) {
			debugging.in_debug_mode = !debugging.in_debug_mode;
			printf("Debug mode %s\n", debugging.in_debug_mode ? "enabled" : "disabled");
		}
	}

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA) {
		current_speed -= 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD) {
		current_speed += 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	current_speed = fmax(0.f, current_speed);
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: HANDLE CHICKEN ROTATION HERE
	// xpos and ypos are relative to the top-left of the window, the chicken's
	// default facing direction is (1, 0)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	Motion& motion = registry.motions.get(pencil);
	motion.position.x = mouse_position.x + 25.f;
	motion.position.y = mouse_position.y - 25.f;
}
