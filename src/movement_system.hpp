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
		}
		else if (key == GLFW_KEY_RIGHT) {
			right = true;
		}
		last = key;
	}

	void release(int key) {
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
};

extern MovementSystem movementSystem;