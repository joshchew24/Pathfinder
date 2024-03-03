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
	just_finished_drawing = true;
	printf("stopping drawing!\n");
}

void DrawingSystem::setDrawPos(const vec2 &pos) {
	drawPos = pos;
}

void build_lines() {
	auto& points = registry.drawnPoints.entities;
	auto& lines = registry.drawnLines.entities;
	if (points.size() < 2)
		return;

	// Only build new lines; don't waste resources on re-building
	if (points.size() - 1 <= lines.size()) // lines are drawn for every point already
	       return;

	for (unsigned int i = lines.size();

	DrawnPoint& prev_point = registry.drawnPoints.get(points[0]);
	DrawnPoint& curr_point = registry.drawnPoints.get(points[1]);

	for (auto& point_ent : points) {
		curr_point = registry.drawnPoints.get(point_ent);
		
		if (prev_point.drawing != curr_point.drawing) {
			prev_point = curr_point;
			continue;
		}
	
		if (curr_point.position != prev_point.position) {
			Entity line = Entity();
			registry.drawnLines.insert(line, 
					{curr_point.drawing, 
					prev_point.position, curr_point.position});
		}

		prev_point = curr_point;
	}

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
	auto to_draw = registry.drawnLines.entities;
	for (auto& line_ent : to_draw) {
		auto& line = registry.drawnLines.get(line_ent);
		createLine(line.p1, vec2(20, 20));
		createLine(line.p2, vec2(20,20));
	}	
}


// Takes 4 bounding box inputs to check against a particular line
bool DrawingSystem::line_collides(Entity& line,
	       	float min_x, float min_y, float max_x, float max_y) {
	// Calculate slope intercept of line
	// Calculate y values bounded by the overlap of (bbox U line) x coords
	// plugged into the line eqn
	// Check if bounded y is within [min_y, max_y]
}
