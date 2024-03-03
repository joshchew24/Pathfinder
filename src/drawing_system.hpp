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

private:
	vec2 drawPos{0,0};
	bool is_drawing = false;
	bool just_finished_drawing = false;
};

extern DrawingSystem drawings;
