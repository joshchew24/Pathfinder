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
	float terminal_velocity;

public:
	const static int LEFT_KEY = GLFW_KEY_A;
	const static int RIGHT_KEY = GLFW_KEY_D;
	bool moving = false;

	MovementSystem() {}

	void init(float friction_arg = config.friction, 
		float move_speed_arg = config.move_speed,
		float terminal_velocity_arg = config.terminal_velocity) {
		friction = friction_arg;
		move_speed = move_speed_arg;
		terminal_velocity = terminal_velocity_arg;
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
		motion.acceleration.x = friction * -motion.velocity.x / abs(motion.velocity.x);
		last = 0;
	}

	void handle_inputs() {
		if (last == 0) return;
		Motion& motion = registry.motions.get(registry.players.entities[0]);
		float dv;
		if (last == LEFT_KEY && left) {
			if (motion.velocity.x >= 0) {
				dv = -move_speed;
			}
			else if (motion.velocity.x < -move_speed) {
				dv = 0;
			}
			else {
				dv = -move_speed - motion.velocity.x;
			}
			motion.velocity.x += dv;
			if (motion.scale.x > 0) {
				motion.scale.x = -motion.scale.x;
			}
			return;
		}
		if (last == RIGHT_KEY && right) {
			if (motion.velocity.x <= 0) {
				dv = move_speed;
			}
			else if (motion.velocity.x > move_speed) {
				dv = 0;
			}
			else {
				dv = move_speed - motion.velocity.x;
			}
			motion.velocity.x += dv;
			motion.velocity.x = move_speed;
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