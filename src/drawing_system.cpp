#include <algorithm>
#include <cmath>

#include "drawing_system.hpp"
#include "world_init.hpp"

DrawingSystem drawings;

// Configuration
static const float pointdraw_freq_ms = 50.f;

DrawingSystem::DrawingSystem() {
}

void DrawingSystem::reset() {
	while (registry.drawnLines.entities.size() > 0)
		registry.remove_all_components_of(registry.drawnLines.entities.back());
	while (registry.drawnPoints.entities.size() > 0)
		registry.remove_all_components_of(registry.drawnPoints.entities.back());
	while (registry.drawings.entities.size() > 0)
		registry.remove_all_components_of(registry.drawings.entities.back());
	map_drawings_points_id.clear();
}


void DrawingSystem::start_drawing() {
	if (!is_drawing) {
		// Create new drawing (and its first point)
		Entity drawing = Entity();
		Entity point = Entity();
		registry.drawings.insert(drawing, {});
		registry.drawnPoints.insert(point, {drawing, drawPos});

		map_drawings_points_id[drawing] = std::vector<Entity>{point};		
		curr_drawing = drawing;
		prev_point = point;
	}

	is_drawing = true;
} 


void DrawingSystem::stop_drawing() {
	// Stop drawing and signal to our system instance to construct lines
	is_drawing = false;
}

void DrawingSystem::set_draw_pos(const vec2 &pos) {
	drawPos = pos;
}

void build_line(Entity& drawing, Entity& p1, Entity& p2) {
	static constexpr float line_width = 10.0f;
	const vec2& p1_pos = registry.drawnPoints.get(p1).position;
	const vec2& p2_pos = registry.drawnPoints.get(p2).position;
	const vec2 dp = p2_pos - p1_pos;
	const float dist = sqrt(dot(dp, dp));

	Motion m;
	m.position = 0.5f * (p1_pos + p2_pos); // midpoint
	m.scale = vec2{dist, line_width};
	m.angle = atan(dp[1] / dp[0]);
	m.fixed = true;

	Entity line = Entity();
	registry.drawnLines.insert(line, {drawing, p1, p2});
	registry.motions.insert(line, m);
	// TODO: Currently re-using debug line render code; maybe find a way to use GL_LINE_STRIP for more continuous lines
	//  might require considerable augmentation of render_system
	registry.renderRequests.insert(line,
				{TEXTURE_ASSET_ID::TEXTURE_COUNT,
				EFFECT_ASSET_ID::EGG,
				GEOMETRY_BUFFER_ID::DRAWN_LINE}); // TODO: make unique drawn line ID
}

void DrawingSystem::step(float elapsed_ms) {
	static float ms_since_last_update = 0;

	// Ensure we're only updating periodically according to freq constant
	ms_since_last_update += elapsed_ms;	
	if (ms_since_last_update < pointdraw_freq_ms)
		return; 	
	ms_since_last_update -= pointdraw_freq_ms;

	if (is_drawing) {
		auto& point_ents = map_drawings_points_id[curr_drawing];
		const DrawnPoint& last_point = registry.drawnPoints.get(prev_point);
		if (last_point.position == drawPos)
			return; // avoid duplicate points
		Entity point = Entity();
		registry.drawnPoints.insert(point, {curr_drawing, drawPos});
		build_line(curr_drawing, prev_point, point);

		point_ents.push_back(point);
		prev_point = point;
	}
}



// Takes 4 bounding box inputs to check against a particular line
bool DrawingSystem::line_collides(Entity& line,
	       	float min_x, float min_y, float max_x, float max_y) {
	const DrawnLine& l = registry.drawnLines.get(line);
	const vec2& line_p1 = registry.drawnPoints.get(l.p1).position;
	const vec2& line_p2 = registry.drawnPoints.get(l.p2).position;

	// get upper/lower bounds on each dimension for the LINE
	const float lower_x = std::min(line_p1[0], line_p2[0]);
	const float upper_x = std::max(line_p1[0], line_p2[0]);
	const float lower_y = std::min(line_p1[1], line_p2[1]);
	const float upper_y = std::max(line_p1[1], line_p2[1]);
	// Determine whether there is intersection
	const bool y_intersects = !(upper_y < min_y || lower_y > max_y);
	const bool x_intersects = !(upper_x < min_x || lower_x > max_x);

	if (!y_intersects || !x_intersects)
		return false;

	const float rise = line_p2[1] - line_p1[1];
	const float run = line_p2[0] - line_p1[0] + 0.000001f; // adding arbitrarily small error to avoid dividing by 0
	const float slope = rise / run;
	const float intercept = line_p2[1] - slope * line_p2[0];
	// Check if line eqn. output is within the bounding box for overlapping x values
	vec2 x_overlap;
	x_overlap[0] = std::max(min_x, lower_x);
	x_overlap[1] = std::min(max_x, upper_x);
	vec2 test_y = slope * x_overlap + intercept;
	// Same for x against y values; we check both to guard against perfect vert/horizontal lines
	vec2 y_overlap;
	y_overlap[0] = std::max(min_y, lower_y);
	y_overlap[1] = std::min(max_y, upper_y);
	vec2 test_x = (1/slope) * (y_overlap - intercept);

	const bool result = (test_y[0] >= min_y && test_y[1] <= max_y) ||
			(test_x[0] >= min_x && test_x[1] <= max_x);

	// DEBUG code
	if (result && debugging.in_debug_mode) { 
		auto& r = registry.renderRequests.get(line);
		r.used_geometry = GEOMETRY_BUFFER_ID::DEBUG_LINE;
	}
	return result;
}

// Debugging function; assumes player bounding box is static w.r.t scale
bool DrawingSystem::check_player_collision(Entity& line) {
	const auto &player = registry.players.entities.back();
	const Motion &m = registry.motions.get(player);

	const float width = abs(m.scale[0]);
	const float height = abs(m.scale[1]);
	vec4 bbox;
	bbox[0] = m.position[0] - width/2;
	bbox[1] = m.position[1] - height/2;
	bbox[2] = m.position[0] + width/2;
	bbox[3] = m.position[1] + height/2;

	return line_collides(line, bbox[0], bbox[1], bbox[2], bbox[3]);
}
