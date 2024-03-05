#pragma once

#include "common.hpp"
#include "components.hpp"

#include "tiny_ecs.hpp"
#include "tiny_ecs_registry.hpp"

class DrawingSystem
{
public:
	DrawingSystem();

	void step(float elapsed_ms);
	
	void start_drawing();
	void stop_drawing();
	void set_draw_pos(const vec2 &pos);
	
	void reset();

	bool check_player_collision(Entity& line);
	bool line_collides(vec2 line_p1, vec2 line_p2, 
			float min_x, float min_y, float max_x, float max_y);

private:
	std::unordered_map<unsigned int, std::vector<Entity>> map_drawings_points_id;
	vec2 drawPos{0,0};
	Entity curr_drawing;
	Entity prev_point;
	bool is_drawing = false;
	bool just_finished_drawing = false;
};

extern DrawingSystem drawings;
