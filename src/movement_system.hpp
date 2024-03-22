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
			Motion& motion = registry.motions.get(registry.players.entities[0]);
			// remove the grounded check to allow air jumps
			if (motion.grounded && motion.jumpsLeft > 0) {
				motion.grounded = false;
				motion.isJumping = true;
				motion.jumpsLeft -= 1;
			}
		}
	}

	void release(int key) {
		if (key == GLFW_KEY_SPACE) {
			Motion& motion = registry.motions.get(registry.players.entities[0]);
			motion.isJumping = false;
			motion.timeJumping = 0.f;
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
		Motion& motion = registry.motions.get(registry.players.entities[0]);
		motion.acceleration.x = FRICTION * -motion.velocity.x / abs(motion.velocity.x);
		last = 0;
	}

	void handle_inputs() {
		if (last == 0) return;
		Motion& motion = registry.motions.get(registry.players.entities[0]);
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
		last = 0;
	}
};

extern MovementSystem movementSystem;