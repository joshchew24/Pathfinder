#pragma once

// internal
#include "common.hpp"

// stlib
#include <set>

class Input {
private:
	std::set<int> active_keys;
public:
	Input() {};
	void press(int key);
	void release(int key);
	std::set<int> get_keys();
};