#pragma once

#include <vector>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"

class AISystem
{
private:
	float elapsed_ms_since_last_update = 0.0f;

public:
	void step(float elapsed_ms);

	bool hasLineOfSight(const vec2& start, const vec2& end);

	void updatePaintCanMovement(const vec2& player_position);

	bool rectangleCollides(const Motion& motion1, const Motion& motion2);

	// Linear intepolation
	template<typename T>
	T lerp(const T& a, const T& b, float t) {
		return (1 - t) * a + t * b;
	}
};