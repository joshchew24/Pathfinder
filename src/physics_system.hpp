#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "config.hpp"

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem
{
private:
	float gravity;
	float terminal_velocity;
	float jump_height;

	void updatePaintCanGroundedState();
	void checkWindowBoundary(Motion& motion);
	void applyFriction(Motion& motion);
public:
	void step(float elapsed_ms);

	PhysicsSystem()	{}

	void init(float gravity_arg = config.gravity, float terminal_velocity_arg = config.terminal_velocity, float jump_height_arg = config.jump_height) {
		gravity = gravity_arg;
		terminal_velocity = terminal_velocity_arg;
		jump_height = jump_height_arg;
	}
};