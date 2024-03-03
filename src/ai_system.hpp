#pragma once

#include <vector>
#include <queue>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include <level_manager.hpp>

struct Node {
    int x, y;
    int g, h;
    Node* parent;

    Node(int x, int y, int g, int h, Node* parent) : x(x), y(y), g(g), h(h), parent(parent) {}

    int f() const {
        return g + h;
    }
};

class AISystem
{
public:
	void init();
	void updateGrid(std::vector<initWall> walls);
	void printGrid();
	void step(float elapsed_ms);
	std::vector<std::pair<int, int>> bestPath(Motion& eMotion, Motion& pMotion);
};