#pragma once

#include "common.hpp"

class Config {
public:
	float gravity;
	float move_speed;
	float jump_height;
	float friction;
	float ink_limit;
	bool line_collision_on;
	float disappearing_timer;
	float paintcan_value;
	float terminal_velocity;
	int starting_level;
	int max_level;
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
		line_collision_on = config["line_collision_on"];
		disappearing_timer = config["disappearing_timer"];
		paintcan_value = config["paintcan_value"];
		terminal_velocity = config["terminal_velocity"];
		starting_level = config["starting_level"];
		max_level = config["max_level"];
		tick_rate = config["tick_rate"];
		target_fps = config["target_fps"];
	}
};

extern Config config;