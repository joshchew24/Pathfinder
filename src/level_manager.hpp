#pragma once

#include <vector>
#include <utility>
#include "common.hpp"
#include <iostream>

struct initWall {
	int x;
	int y;
	int xSize;
	int ySize;
};

class Level {
public:
	std::vector<initWall> walls;
	std::pair<float, float> checkpoint;
	std::pair<float, float> endPoint;
	std::pair<float, float> playerPos;
};

class LevelManager {
public:
	std::vector<Level> levels;

	void initLevel() {
		//level 1
		initWall w = { 900, window_height_px - 90, 1000, 600 };
		initWall w1 = { window_width_px - window_width_px, window_height_px - 100, 400, 400 };
		initWall w2 = { window_width_px - 200, window_height_px - 60, 500, 400 };
		Level Level1;
		Level1.walls.push_back(w);
		Level1.walls.push_back(w1);
		Level1.walls.push_back(w2);
		Level1.checkpoint.first = window_width_px - 300;
		Level1.checkpoint.second = window_height_px - 305;
		Level1.endPoint.first = window_width_px - 200;
		Level1.endPoint.second = window_height_px - 305;
		Level1.playerPos.first = window_width_px / 2;
		Level1.playerPos.second = 460;
		levels.push_back(Level1);

		Level Level2;

		int startY = window_height_px - 200;
		int stairWidth = 100;
		int stairHeight = 20;
		int numStairs = 10;
		int stairGap = 50;
		int startX = (window_width_px - numStairs * (stairWidth + stairGap)) / 2;


		createStairs(numStairs, Level2, startX, startY, stairWidth, stairGap, stairHeight);

		Level2.checkpoint.first = startX + (numStairs / 2) * (stairWidth + stairGap) + 10;
		Level2.checkpoint.second = startY - (numStairs / 2) * (stairHeight + stairGap) - (stairHeight / 2) - 55;

		Level2.playerPos.first = startX;
		Level2.playerPos.second = startY - (numStairs - 1) * (stairHeight + stairGap) - stairHeight;

		Level2.endPoint.first = startX + (numStairs - 1) * (stairWidth + stairGap) + 10;
		Level2.endPoint.second = startY - (numStairs - 1) * (stairHeight + stairGap) - 65;

		levels.push_back(Level2);

		Level Level3;
		initWall w_3 = { window_width_px - 500, window_height_px - 200, 1000, 600 };


		Level3.walls.push_back(w_3);
		Level3.walls.push_back(w1);

		stairWidth = 150; 
		stairHeight = 20;
		numStairs = 3;
		stairGap = 80;

		startX = w1.x + w1.xSize;
		startY = w1.y - 2 * stairHeight - 200;

		createStairs(numStairs, Level3, startX, startY, stairWidth, stairGap, stairHeight);

		Level3.checkpoint.first = Level3.walls.back().x + Level3.walls.back().xSize / 2 - 10;
		Level3.checkpoint.second = Level3.walls.back().y - Level3.walls.back().ySize / 2 - 55;

		Level3.playerPos.first = 100;
		Level3.playerPos.second = window_height_px - 400;

		Level3.endPoint.first = w_3.x + w_3.xSize / 2 - 100;
		Level3.endPoint.second = w_3.y - w_3.ySize / 2 - 55;

		levels.push_back(Level3);
	}

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
			initWall stair;
			stair.x = startX + i * (stairWidth + stairGap);
			stair.y = startY - i * (stairHeight + stairGap);
			stair.xSize = stairWidth;
			stair.ySize = stairHeight;
			level.walls.push_back(stair);
		}
	}
};
