#pragma once

#include <vector>
#include <utility>
#include <iostream>

#include "common.hpp"
#include "config.hpp"

// data to generate walls
struct InitWall {
	int x;
	int y;
	int xSize;
	int ySize;
};

struct InitSpike {
	int x;
	int y;
	float angle;
};

struct InitPaintCan {
	vec2 pos;
	float value;
};

struct InitBoulderSpawner {
	vec2 pos;
	bool random_x;
	float delay;
};

struct InitSpikeProjectileSpawner {
	vec2 pos;
	vec2 size;
	vec2 vel;
	float angle;
	float delay;
};

struct Level {
	std::vector<InitWall> walls;
	std::vector<InitSpike> spikes;
	bool hasCheckpoint = false;
	vec2 checkpoint;
	vec2 endPoint;
	bool hasHint = false;
	vec2 hintPos;
	std::string hint;
	vec2 hintTextPos;
	float gravity;
	float friction;
	float inkLimit;
	vec2 playerSpawn;
	std::vector<InitBoulderSpawner> boulderSpawners;
	std::vector<InitSpikeProjectileSpawner> spikeProjectileSpawners;
	bool hasChaseBoulder = false;
	vec2 chaseBoulder;
	std::vector<vec2> archers;
	std::vector<InitPaintCan> paintcans;
};

class LevelManager {
private:
	InitWall parseWall(json wallJson);
	void parseStair(Level& level, json stairJson);
	void parseSpike(Level& level, json spikeJson);
	InitPaintCan LevelManager::parsePaintCan(json paintcanJson);
	InitBoulderSpawner LevelManager::parseBoulderSpawner(json boulderSpawnerJson);
	InitSpikeProjectileSpawner LevelManager::parseSpikeProjectileSpawner(json spikeProjectileSpawnerJson);

public:
	std::vector<Level> levels;
	
	void loadLevels();
	Level loadLevel(int levelNumber);

	void printLevelsInfo() {
		std::cout << "Number of levels: " << levels.size() << std::endl;
		for (const auto& level : levels) {
			std::cout << "Level Info:" << std::endl;
			std::cout << "  Walls:" << std::endl;
			for (const auto& wall : level.walls) {
				std::cout << "    x: " << wall.x << ", y: " << wall.y
					<< ", xSize: " << wall.xSize << ", ySize: " << wall.ySize << std::endl;
			}
			std::cout << "  Checkpoint: (" << level.checkpoint.x << ", " << level.checkpoint.y << ")" << std::endl;
		}
	}
};
