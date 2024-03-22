#pragma once

// internal
#include "common.hpp"
#include "config.hpp"

// stlib
#include <deque>

#include "tiny_ecs_registry.hpp"

class MovementSystem {
private:
	bool left = false;
	bool right = false;
	int last = 0;
	float friction;

public:
	const static int LEFT_KEY = GLFW_KEY_A;
	const static int RIGHT_KEY = GLFW_KEY_D;

	MovementSystem() {}

	void init(float friction_arg = config.friction) {
		printf("%f\n", friction_arg);
		friction = friction_arg;
	}

	void press(int key) {
		if (key == LEFT_KEY) {
			left = true;
			last = key;
		}
		else if (key == RIGHT_KEY) {
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
		if (key == LEFT_KEY) {
			left = false;
			if (right) {
				last = RIGHT_KEY;
				return;
			}
		}
		else if (key == RIGHT_KEY) {
			right = false;
			if (left) {
				last = LEFT_KEY;
				return;
			}
		}
		// if no directional keys are pressed, apply horizontal deceleration
		Motion& motion = registry.motions.get(registry.players.entities[0]);
		motion.acceleration.x = friction * -motion.velocity.x / abs(motion.velocity.x);
		last = 0;
	}

	void handle_inputs() {
		if (last == 0) return;
		Motion& motion = registry.motions.get(registry.players.entities[0]);
		motion.acceleration.x = 0.0;
		float vel = 250.f;
		if (last == LEFT_KEY && left) {
			motion.velocity.x = vel * -1.f;
			if (motion.scale.x > 0) {
				motion.scale.x = -motion.scale.x;
			}
			return;
		}
		if (last == RIGHT_KEY && right) {
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