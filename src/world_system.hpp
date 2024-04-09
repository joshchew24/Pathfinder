#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "render_system.hpp"
#include "level_manager.hpp"
#include "ai_system.hpp"

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{
public:
	//level index
	int level_idx;

	WorldSystem();

	// Creates a window
	GLFWwindow* create_window();

	// starts the game
	void init(RenderSystem* renderer, bool* mainMenu);

	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);

	// Linear intepolation
	template<typename T>
	T lerp(const T& a, const T& b, float t) {
		return (1 - t) * a + t * b;
	}

	// Check for collisions
	void handle_collisions(float elapsed_ms);

	// Should the game be over ?
	bool is_over()const;

	void displayTutorialImage();

	void createLevel();

	LevelStruct createLevelStruct(int level_idx);

	void next_level();
private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);
	void on_mouse_click(int button, int action, int mod);

	// Save/Load helpers
	void save_checkpoint();
	void load_checkpoint();

	// restart level
	void restart_game();

	void handlePlayerAnimation(float elapsed_ms_since_last_update);
	
	void handleLineCollision(const Entity& line, float elapsed_ms);

	void createIndividualPlatforms(vec2 position, vec2 size);

	void drawLinesLevel4(int currDrawing);

	void switchHintAnimation(Entity e, float elapsedTime);

	void createDrawOnLines(int x, int y, float rotation);

	LevelManager levelManager;
	int maxLevel = 8;
	LevelStruct level;

	//AI
	AISystem aiSystem;
	//advanced AI
	Entity chaseBoulder;
	int currentNode = 0;
	std::vector<std::pair<int, int>> bestPath;
	float speed = 0.01f;
	const int gridSize = 30;
	int FrameInterval = 20;
	int FrameCount = 0;

	// OpenGL window handle
	GLFWwindow* window;

	// Game state
	RenderSystem* renderer;
	float current_speed;
	float next_boulder_spawn;
	Entity player;
	Entity pencil;
	Entity tutorial;

	//To set running animation
	int currentRunningTexture = (int) TEXTURE_ASSET_ID::OLIVER;
	int elapsedMsTotal = 0;

	// music references
	Mix_Music* background_music;
	Mix_Chunk* dead_sound;
	Mix_Chunk* checkpoint_sound;
	Mix_Chunk* level_win_sound;
	Mix_Chunk* ink_pickup_sound;
	bool checkPointAudioPlayer = false;

	//camera speed
	float cameraSpeed = 0.004f;

	//platform disappear time
	float level4DisappearTimer = 3500;
	bool level4Disappeared = false;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

	int currDrawing = 0;
	bool currDrawn = false;

	//hint animation index
	int currentHintTexture = (int)TEXTURE_ASSET_ID::HINT1;
	int hintElapsedMsTotal = 0;

	bool* mainMenu;
	bool restart = false;
	bool resume = false;
	bool exit = false;

	Entity mainMenuEntity;

};
