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

struct Spike {
	int x;
	int y;
	float angle;
};

struct InitPaintCan {
	int x;
	int y;
	float value;
};

struct LevelStruct {
	std::vector<InitWall> walls;
	std::vector<Spike> spikes;
	glm::vec2 checkpoint;
	glm::vec2 endPoint;
	glm::vec2 hintPos;
	std::string hint;
	glm::vec2 hintTextPos;
	float gravity;
	float friction;
	float inkLimit;
	glm::vec2 playerSpawn;
	std::vector<glm::vec2> boulderSpawns;
	std::vector<glm::vec2> chaseBoulderSpawns;
	std::vector<glm::vec2> archerSpawns;
	std::vector<InitPaintCan> paintCanSpawns;
};

class Level {
public:
	std::vector<InitWall> walls;
	std::pair<float, float> checkpoint;
	std::pair<float, float> endPoint;
	std::pair<float, float> playerPos;
	std::vector<Spike> spikes;
	std::pair<float, float> hintPos;
	std::string hint;
	std::pair<float, float> hintTextPos;
};

class LevelManager {
private:
	int numLevels = 1;
	InitWall parseWall(json wallJson);
	void parseStair(LevelStruct level, json stairJson);
	void parseSpike(LevelStruct level, json spikeJson);
public:
	std::vector<Level> levels;
	std::vector<LevelStruct> structLevels;
	
	void loadLevels();
	LevelStruct loadLevel(int levelNumber);
	void initLevel();

	void printLevelsInfo() {
		std::cout << "Number of levels: " << levels.size() << std::endl;
		for (const auto& level : levels) {
			std::cout << "Level Info:" << std::endl;
			std::cout << "  Walls:" << std::endl;
			for (const auto& wall : level.walls) {
				std::cout << "    x: " << wall.x << ", y: " << wall.y
					<< ", xSize: " << wall.xSize << ", ySize: " << wall.ySize << std::endl;
			}
			std::cout << "  Checkpoint: (" << level.checkpoint.first << ", " << level.checkpoint.second << ")" << std::endl;
		}
	}

	void createStairs(int numStairs, Level& level, int startX, int startY, int stairWidth, int stairGap, int stairHeight) {
		for (int i = 0; i < numStairs; ++i) {
			InitWall stair;
			stair.x = startX + i * (stairWidth + stairGap);
			stair.y = startY - i * (stairHeight + stairGap);
			stair.xSize = stairWidth;
			stair.ySize = stairHeight;
			level.walls.push_back(stair);
		}
	}

	void createSpikes(Level& level, int x, int y, int numOfSpikes, int gap, float angle) {
		for (int i = 0; i < numOfSpikes; ++i) {
			Spike spike;
			spike.x = x + i * gap;
			spike.y = y;
			spike.angle = angle;
			level.spikes.push_back(spike);
		}
	}
};
