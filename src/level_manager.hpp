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

		//level 2
		initWall l2w1 = { 900, window_height_px - 90, 1000, 600 };
		Level Level2;
		Level2.walls.push_back(l2w1);
		Level2.playerPos.first = window_width_px / 2;
		Level2.playerPos.second = 460;
		Level2.checkpoint.first = 760;
		Level2.checkpoint.second = 500;
		Level2.endPoint.first = 1200;
		Level2.endPoint.second = 500;
		levels.push_back(Level2);
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
};
