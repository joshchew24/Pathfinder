#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "config.hpp"
#include <collision_system.hpp>

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem
{
private:
	float gravity;
	float terminal_velocity;
	float jump_height;
	CollisionSystem collisionSystem;
	void checkWindowBoundary(Motion& motion);

	void updateArrowPosition(Motion& arrowMotion, Motion& playerMotion, float deltaTime, float flightDuration, BezierProjectile& projectile, Entity projectileEntity);
	using UpdateGroundedStateFunc = std::function<void(Motion& motion, bool isGrounded)>;
	template<typename T>
	void updateGroundedStateForEntities(ECSRegistry& registry, const ComponentContainer<T>& entityContainer, UpdateGroundedStateFunc updateFunc);
  
	void applyFriction(Motion& motion, float elapsed_ms);
public:
	void step(float elapsed_ms);

	PhysicsSystem()	{}

	void init(float gravity_arg = config.gravity, float terminal_velocity_arg = config.terminal_velocity, float jump_height_arg = config.jump_height) {
		gravity = gravity_arg;
		terminal_velocity = terminal_velocity_arg;
		jump_height = jump_height_arg;

		collisionSystem.init();
	}
};