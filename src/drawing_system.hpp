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
	void setDrawPos(const vec2 &pos);
	
	void drawLines();

	bool line_collides(Entity& line, float min_x, float min_y, float max_x, float max_y);

private:
	vec2 drawPos{0,0};
	bool is_drawing = false;
	bool just_finished_drawing = false;
};

extern DrawingSystem drawings;
