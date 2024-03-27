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
		// Create new drawing (and its first point + perpendicular "capstop")
		Entity drawing = Entity();
		Entity point = Entity();
		Entity capstop = Entity();
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
	const vec2& pos1 = registry.drawnPoints.get(p1).position;
	const vec2& pos2 = registry.drawnPoints.get(p2).position;
	const vec2 dp = pos2 - pos1; // displacement
	const float dist = sqrt(dot(dp, dp));
	
	Entity line = Entity();
	DrawnLine &dline = registry.drawnLines.emplace(line);

	Motion &m = registry.motions.emplace(line);
	m.position = 0.5f * (pos1 + pos2); // midpoint
	m.scale = vec2{dist, dline.line_width};
	m.angle = atan2(dp[1], dp[0]);
	m.fixed = true;
	
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

	registry.renderRequests.insert(line,
				{TEXTURE_ASSET_ID::TEXTURE_COUNT,
				EFFECT_ASSET_ID::EGG,
				GEOMETRY_BUFFER_ID::DRAWN_LINE}); 
	return line;
}

void build_joint(Entity& drawing, Entity& l1, Entity& l2) {
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

	Motion &m = registry.motions.emplace(j);
	m.scale = vec2(line1.line_width / 2.f, line1.line_width / 2.f);
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



