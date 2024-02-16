#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem
{
public:
	void step(float elapsed_ms);

	PhysicsSystem()
	{
	}

private: 
	float gravityConstant = 9.8f;
	const float TERMINAL_VELOCITY = 250.f;
};