#pragma once

#include "common.hpp"

class Config {
public:
	float gravity;
	float move_speed;
	float jump_height;
	float friction;
	float ink_limit;
	float default_paintcan_value;
	float terminal_velocity;
	int starting_level;
	int tick_rate;
	int target_fps;

	void load(std::string file_name="config.json") {
		std::ifstream file(data_path() + "/" + file_name);
		json config = json::parse(file);

		gravity = config["gravity"];
		move_speed = config["move_speed"];
		jump_height = config["jump_height"];
		friction = config["friction"];
		ink_limit = config["ink_limit"];
		default_paintcan_value = config["default_paintcan_value"];
		terminal_velocity = config["terminal_velocity"];
		starting_level = config["starting_level"];
		tick_rate = config["tick_rate"];
		target_fps = config["target_fps"];
	}
};

extern Config config;