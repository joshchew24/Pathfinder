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
	float move_speed;

public:
	const static int LEFT_KEY = GLFW_KEY_A;
	const static int RIGHT_KEY = GLFW_KEY_D;

	MovementSystem() {}

	void init(float friction_arg = config.friction, 
		float move_speed_arg = config.move_speed) {
		friction = friction_arg;
		move_speed = move_speed_arg;
	}

	// if player is moving left or right
	bool leftOrRight() { return left || right; }

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
		int vel_dir = motion.velocity.x / abs(motion.velocity.x);
		float da = friction * -vel_dir;
		if (motion.acceleration.x * da > 0) { // only add force up to friction
			da -= motion.acceleration.x;
		}
		motion.acceleration.x += da;
		last = 0;
	}

	void handle_inputs() {
		if (last == 0) return;
		Motion& motion = registry.motions.get(registry.players.entities[0]);
		int dir;
		if (last == LEFT_KEY && left) {
			dir = -1;
			if (motion.scale.x > 0) {
				motion.scale.x = -motion.scale.x;
			}
		}
		else if (last == RIGHT_KEY && right) {
			dir = 1;
			if (motion.scale.x < 0) {
				motion.scale.x = -motion.scale.x;
			}
		}
		float dv = move_speed * dir;
		if (motion.velocity.x * dv > 0) { // only add force up to move_speed
			dv -= motion.velocity.x;
		}
		motion.velocity.x += dv;
	}
	
	void reset() {
		left = false;
		right = false;
		last = 0;
	}
};

extern MovementSystem movementSystem;