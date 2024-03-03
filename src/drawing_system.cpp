#include "drawing_system.hpp"
#include "world_init.hpp"

DrawingSystem drawings;

// Configuration
static const float pointdraw_freq_ms = 50.f;

DrawingSystem::DrawingSystem() {
}


// DrawingSystem::~DrawingSystem() {
// 	// Destroy all points and lines
// }
//
// DrawingSystem::reset() {
// }


void DrawingSystem::start_drawing() {
	// Create new drawing
	Entity drawing = Entity();
	registry.drawings.insert(drawing, {});
	is_drawing = true;
	printf("starting drawing!\n");
} 


void DrawingSystem::stop_drawing() {
	// Stop drawing and signal to our system instance to construct lines
	is_drawing = false;
	printf("stopping drawing!\n");
}

void DrawingSystem::setDrawPos(const vec2 &pos) {
	drawPos = pos;
}

void build_lines() {
	if (registry.drawnPoints.entities.size() < 2)
		return;
	Entity &p1 = NULL;
	Entity &p2 = NULL;

	printf("making lines...\n");
}

void DrawingSystem::step(float elapsed_ms) {
	static float ms_since_last_update = 0;

	// Ensure we're only updating periodically according to freq constant
	ms_since_last_update += elapsed_ms;	
	if (ms_since_last_update < pointdraw_freq_ms)
		return; 	
	ms_since_last_update -= pointdraw_freq_ms;

	if (just_finished_drawing) {
		build_lines();
		just_finished_drawing = false;
	}
	if (is_drawing) {
		auto drawing = registry.drawings.entities.back();
		Entity point = Entity();
		registry.drawnPoints.insert(point, {drawing, drawPos});
	}
}


void DrawingSystem::drawLines() {
	auto to_draw = registry.drawnPoints.entities;
	for (auto& line : to_draw) {
		auto& point = registry.drawnPoints.get(line);
		createLine(point.position, vec2(20,20));	
	}	
}
