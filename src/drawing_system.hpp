#pragma once

#include "common.hpp"
#include "components.hpp"

#include "tiny_ecs.hpp"
#include "tiny_ecs_registry.hpp"

class DrawingSystem
{
public:
	DrawingSystem();

	void step(float elapsed_ms, bool lineCollisionOn);
	
	void start_drawing();
	void stop_drawing();
	void set_draw_pos(const vec2 &pos);
	
	void reset(float inkLimit);

	bool check_player_collision(Entity& line);

	bool currently_drawing() { return is_drawing; }

	void add_drawing_count(float count);

	float get_drawing_count();

	static float remainingDrawingCount;

	Entity get_prev_line() { return prev_line; }

private:
	std::unordered_map<unsigned int, std::vector<Entity>> map_drawings_points_id;
	vec2 drawPos{0,0};
	Entity curr_drawing;
	Entity prev_point;
	Entity prev_line;
	bool is_drawing = false;
	bool just_finished_drawing = false;
	bool start_connecting = false;
};

extern DrawingSystem drawings;
