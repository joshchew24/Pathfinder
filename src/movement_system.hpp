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
public:
	MovementSystem() {};

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
		last = 0;
	}

	void handle_inputs() {
		if (last == 0) return;
		if (last == GLFW_KEY_LEFT && left) {
			printf("go left\n");
			return;
		}
		if (last == GLFW_KEY_RIGHT && right) {
			printf("go right\n");
			return;
		}
	}
};

extern MovementSystem movementSystem;