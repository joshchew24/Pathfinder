#include "drawing_system.hpp"
#include "world_init.hpp"

DrawingSystem drawings;

// Configuration
static const float pointdraw_freq_ms = 50.f;

DrawingSystem::DrawingSystem() {
}

void DrawingSystem::reset() {
	registry.drawnLines.clear();
	registry.drawnPoints.clear();
	registry.drawings.clear();
}


void DrawingSystem::start_drawing() {
	// Create new drawing (and its first point)
	Entity drawing = Entity();
	unsigned int point_idx = registry.drawnPoints.entities.size();

	registry.drawings.insert(drawing, {point_idx, 0, 1});
	is_drawing = true;
	printf("starting drawing!\n");
} 


void DrawingSystem::stop_drawing() {
	// Stop drawing and signal to our system instance to construct lines
	is_drawing = false;
	just_finished_drawing = true; // TODO: replace this mechanism with rendering while drawing
	printf("stopping drawing!\n");
}

void DrawingSystem::setDrawPos(const vec2 &pos) {
	drawPos = pos;
}

void build_lines() {
	for (auto& drawing_ent : registry.drawings.entities) {
		Drawing& drawing = registry.drawings.get(drawing_ent);
		if (drawing.drawn_points >= drawing.num_points) 
			continue; // don't do redundant work
		if (drawing.num_points < 2)
			continue; // not enough points in drawing to make a line

		size_t first = drawing.first_point_idx;
		size_t last = first + drawing.num_points;

		// Hold indexes into point component array
		size_t prev_idx = first;
		for (size_t point_idx = first + 1; point_idx < last; point_idx++) {
			Entity line_ent = Entity();
			registry.drawnLines.insert(line_ent, {drawing_ent, prev_idx, point_idx});	
			prev_idx = point_idx;
			drawing.drawn_points++;
			printf("drawn points: %d\n", drawing.drawn_points);
		}
	}
}

void DrawingSystem::step(float elapsed_ms) {
	static float ms_since_last_update = 0;

	// Ensure we're only updating periodically according to freq constant
	ms_since_last_update += elapsed_ms;	
	if (ms_since_last_update < pointdraw_freq_ms)
		return; 	
	ms_since_last_update -= pointdraw_freq_ms;

	if (just_finished_drawing) {
		// Drawing finished trigger; do whatever u want here
		just_finished_drawing = false;
	}
	if (is_drawing) {
		auto& drawing_ent = registry.drawings.entities.back();	
		auto& drawing = registry.drawings.get(drawing_ent);
		auto& points = registry.drawnPoints.components;
		if (points.size() != 0 && points.back().position == drawPos)
			return; // avoid duplicate points

		Entity point = Entity();
		registry.drawnPoints.insert(point, {drawing_ent, drawPos});
		drawing.num_points++;
		printf("# points: %d\n", drawing.num_points);
	}

	build_lines();
}


void DrawingSystem::drawLines() {
	auto to_draw = registry.drawnLines.entities;
	vec2 rect{20,20};
 	for (auto& line_ent : to_draw) {
		DrawnLine& line = registry.drawnLines.get(line_ent);
		auto& p1 = registry.drawnPoints.components[line.p1_idx];
		auto& p2 = registry.drawnPoints.components[line.p2_idx];
		createLine(p1.position, rect);
		createLine(p2.position, rect);
 	}
	
}


// Takes 4 bounding box inputs to check against a particular line
bool DrawingSystem::line_collides(Entity& line,
	       	float min_x, float min_y, float max_x, float max_y) {
	// Calculate slope intercept of line
	// Calculate y values bounded by the overlap of (bbox U line) x coords
	// plugged into the line eqn
	// Check if bounded y is within [min_y, max_y]
	return false;
}
