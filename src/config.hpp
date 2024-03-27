#pragma once

#include "common.hpp"

class Config {
public:
	float gravity;
	float move_speed;
	float jump_height;
	float friction;
	float terminal_velocity;
	int starting_level;

	void load(std::string file_name="config.json") {
		std::ifstream file(data_path() + "/" + file_name);
		json config = json::parse(file);

		gravity = config["gravity"];
		move_speed = config["move_speed"];
		jump_height = config["jump_height"];
		friction = config["friction"];
		terminal_velocity = config["terminal_velocity"];
		starting_level = config["starting_level"];
	}
};

extern Config config;