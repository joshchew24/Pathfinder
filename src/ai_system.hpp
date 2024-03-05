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
private:
	float elapsed_ms_since_last_update = 0.0f;

public:
	void init();
	void updateGrid(std::vector<initWall> walls);
	void printGrid();
	void step(float elapsed_ms);
	std::vector<std::pair<int, int>> bestPath(Motion& eMotion, Motion& pMotion);

	bool hasLineOfSight(const vec2& start, const vec2& end);

	void updatePaintCanMovement(const vec2& player_position);

	bool rectangleCollides(const Motion& motion1, const Motion& motion2);

	// Linear intepolation
	template<typename T>
	T lerp(const T& a, const T& b, float t) {
		return (1 - t) * a + t * b;
	}
};