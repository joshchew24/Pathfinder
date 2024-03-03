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
	const static int LEFT_KEY = GLFW_KEY_A;
	const static int RIGHT_KEY = GLFW_KEY_D;
	MovementSystem() {}

	void press(int key) {
		if (key == LEFT_KEY) {
			left = true;
		}
		else if (key == RIGHT_KEY) {
			right = true;
		}
		last = key;
	}

	void release(int key) {
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
		// we only reach this case if no button is pressed
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