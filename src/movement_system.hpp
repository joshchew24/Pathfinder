#pragma once

// internal
#include "common.hpp"

// stlib
#include <deque>

#include "tiny_ecs_registry.hpp"

class MovementSystem {
private:
	bool left = false;
	bool right = false;
	bool space = false;
	std::chrono::steady_clock::time_point space_pressed;
	int last = 0;
	const float FRICTION = 5.f;

public:
	MovementSystem() {}

	void press(int key) {
		if (key == GLFW_KEY_LEFT) {
			left = true;
			last = key;
		}
		else if (key == GLFW_KEY_RIGHT) {
			right = true;
			last = key;
		}
		else if (key == GLFW_KEY_SPACE) {
			space = true;
			space_pressed = Clock::now();
		}
	}

	void release(int key) {
		if (key == GLFW_KEY_SPACE) {
			space = false;
			auto t = Clock::now();
			float time_held = (float)(std::chrono::duration_cast<std::chrono::microseconds>(t - space_pressed)).count() / 1000;
			float jump_height_multiplier = clamp(time_held, 100.f, 500.f) / 500.f;

			Entity player = registry.players.entities[0];
			auto& motions = registry.motions;
			Motion& motion = motions.get(player);
			if (motion.grounded && !registry.deathTimers.has(player)) {
				motion.velocity.y = -600.f * jump_height_multiplier;
				motion.grounded = false;
			}
			return;
		}
		if (key == GLFW_KEY_LEFT) {
			left = false;
			if (right) {
				last = GLFW_KEY_RIGHT;
				return;
			}
		}
		else if (key == GLFW_KEY_RIGHT) {
			right = false;
			if (left) {
				last = GLFW_KEY_LEFT;
				return;
			}
		}
		// if no directional keys are pressed, apply horizontal deceleration
		Entity& player = registry.players.entities[0];
		auto& motions = registry.motions;
		Motion& motion = motions.get(player);
		motion.acceleration.x = FRICTION * -motion.velocity.x / abs(motion.velocity.x);
		last = 0;
	}

	void handle_inputs() {
		if (last == 0) return;
		Entity& player = registry.players.entities[0];
		auto& motions = registry.motions;
		Motion& motion = motions.get(player);
		motion.acceleration.x = 0.0;
		float vel = 250.f;
		if (last == GLFW_KEY_LEFT && left) {
			motion.velocity.x = vel * -1.f;
			if (motion.scale.x > 0) {
				motion.scale.x = -motion.scale.x;
			}
			return;
		}
		if (last == GLFW_KEY_RIGHT && right) {
			motion.velocity.x = vel;
			if (motion.scale.x < 0) {
				motion.scale.x = -motion.scale.x;
			}
			return;
		}
	}
	
	void reset() {
		left = false;
		right = false;
		space = false;
		last = 0;
	}
};

extern MovementSystem movementSystem;