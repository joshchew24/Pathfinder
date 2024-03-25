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
		start_connecting = false;
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

Entity build_line(Entity& drawing, Entity& p1, Entity& p2) {
	static constexpr float line_width = 10.0f;
	const vec2& pos1 = registry.drawnPoints.get(p1).position;
	const vec2& pos2 = registry.drawnPoints.get(p2).position;
	const vec2 dp = pos2 - pos1; // displacement
	const float dist = sqrt(dot(dp, dp));
	
	Entity line = Entity();

	Motion &m = registry.motions.emplace(line);
	m.position = 0.5f * (pos1 + pos2); // midpoint
	m.scale = vec2{dist, line_width};
	m.angle = atan2(dp[1], dp[0]);
	m.fixed = true;
	
	DrawnLine &dline = registry.drawnLines.emplace(line);
	dline.drawing = drawing;
	dline.p1 = p1;
	dline.p2 = p2;
	// get upper/lower bounds on each dimension for the line
	dline.x_bounds = (pos1.x < pos2.x) ? vec2(pos1.x, pos2.x) : vec2(pos2.x, pos1.x);
	dline.y_bounds = (pos1.y < pos2.y) ? vec2(pos1.y, pos2.y) : vec2(pos2.y, pos1.y);
	// get line equation coeffs
	const float rise = pos2.y - pos1.y;
	const float run = pos2.x - pos1.x + 0.000001f; // adding arbitrarily small error to avoid dividing by 0
	dline.slope = rise / run;
	dline.intercept = pos2.y - dline.slope * pos2.x;

	// TODO: Currently re-using debug line render code; maybe find a way to use GL_LINE_STRIP for more continuous lines
	//  might require considerable augmentation of render_system
	registry.renderRequests.insert(line,
				{TEXTURE_ASSET_ID::TEXTURE_COUNT,
				EFFECT_ASSET_ID::EGG,
				GEOMETRY_BUFFER_ID::DRAWN_LINE}); // TODO: make unique drawn line ID
	return line;
}

void build_joint(Entity& drawing, Entity& l1, Entity& l2) {
	static constexpr float line_width = 10.f;
	Entity j = Entity();
	DrawnJoint& joint = registry.drawnJoints.emplace(j);
	Motion& l1_m = registry.motions.get(l1);
	Motion& l2_m = registry.motions.get(l2);
	DrawnLine& line1 = registry.drawnLines.get(l1);
	DrawnPoint& p2 = registry.drawnPoints.get(line1.p2);
	if (l1_m.angle == l2_m.angle)
		return;
	joint.drawing = drawing;
	joint.l1 = l1;
	joint.l2 = l2;
	joint.top_angle = M_PI - (l2_m.angle - l1_m.angle);

	Motion &m = registry.motions.emplace(j);
	m.scale = vec2(5,5);
	m.fixed = true;
	m.angle = 1.5 * M_PI + l1_m.angle;
	m.position = p2.position;
	m.position += vec2(5*cos(m.angle), 5*sin(m.angle));	
	registry.renderRequests.insert(j,
				{TEXTURE_ASSET_ID::TEXTURE_COUNT,
				EFFECT_ASSET_ID::EGG,
				GEOMETRY_BUFFER_ID::JOINT_TRIANGLE}); 
}

void DrawingSystem::step(float elapsed_ms) {
	static float ms_since_last_update = 0;

	// DEBUG: check player static bbox collision with every drawn line
	if (debugging.in_debug_mode)
		for (auto &line : registry.drawnLines.entities)
			check_player_collision(line);

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
		Entity line = build_line(curr_drawing, prev_point, point);

		if (!start_connecting) {
			prev_line = line;
			start_connecting = true;
		}
		else {
			build_joint(curr_drawing, prev_line, line);
			prev_line = line;
		}
		point_ents.push_back(point);
		prev_point = point;

	}
}



// Takes 4 bounding box inputs to check against a particular line
bool DrawingSystem::line_collides(Entity& line,
	       	float min_x, float min_y, float max_x, float max_y) {
	const DrawnLine& dline = registry.drawnLines.get(line);

	// Determine whether there is intersection
	const bool y_intersects = !(dline.y_bounds[1] < min_y || dline.y_bounds[0] > max_y);
	const bool x_intersects = !(dline.x_bounds[1] < min_x || dline.x_bounds[0] > max_x);

	if (!y_intersects || !x_intersects)
		return false;

	// Check if line eqn. output is within the bounding box for overlapping x values
	vec2 x_overlap(std::max(min_x, dline.x_bounds[0]),  std::min(max_x, dline.x_bounds[1]));
	vec2 test_y = dline.slope * x_overlap + dline.intercept;
	// Same for x against y values; we check both to guard against perfect vert/horizontal lines
	vec2 y_overlap(std::max(min_y, dline.y_bounds[0]),  std::min(max_y, dline.y_bounds[1]));
	vec2 test_x = (1 / dline.slope) * (y_overlap - dline.intercept);

	const bool result = (test_y[0] >= min_y && test_y[1] <= max_y) ||
			    (test_x[0] >= min_x && test_x[1] <= max_x);

	// DEBUG code
	if (result && debugging.in_debug_mode) { 
		auto& r = registry.renderRequests.get(line);
		r.used_geometry = GEOMETRY_BUFFER_ID::DEBUG_LINE; // turn the line red
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
