#include <collision_system.hpp>
#include "world_init.hpp"


// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

bool CollisionSystem::collides(const Motion& motion1, const Entity& entity1, const Motion& motion2, const Entity& entity2)
{
	if (registry.renderRequests.has(entity1) && registry.renderRequests.get(entity1).used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT &&
		registry.meshPtrs.has(entity1) && registry.meshPtrs.get(entity1)->vertices.size() > 0) {
		return SATcollision(registry.meshPtrs.get(entity1), motion1, motion2);
	}
	else if (registry.renderRequests.has(entity2) && registry.renderRequests.get(entity2).used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT &&
		registry.meshPtrs.has(entity2) && registry.meshPtrs.get(entity2)->vertices.size() > 0) {
		return SATcollision(registry.meshPtrs.get(entity2), motion2, motion1);
	}
	else {
		return rectangleCollides(motion1, motion2);
	}
	return false;
}

ColoredVertex v0;

// find vertex next to top in stack
ColoredVertex nextToTop(std::stack<ColoredVertex>& S) {
	ColoredVertex v = S.top();
	S.pop();
	ColoredVertex res = S.top();
	S.push(v);
	return res;
}

// swap positions in vector array
void swap(ColoredVertex& v1, ColoredVertex& v2) {
	ColoredVertex temp = v1;
	v1 = v2;
	v2 = temp;
}

// find distance between two points
float distSq(vec3 p1, vec3 p2) {
	return (p1.x - p2.x) * (p1.x - p2.x) +
		(p1.y - p2.y) * (p1.y - p2.y);
}

// find orientation of triplet vertices, where 0 == collinear, 1 == clockwise, 2 == counterclockwise
int orientation(vec3 p, vec3 q, vec3 r) {
	float threshold = 1e-5;
	float val = (q.y - p.y) * (r.x - q.x) -
		(q.x - p.x) * (r.y - q.y);
	if (abs(val) <= threshold) return 0; // collinear
	return (val > 0) ? 1 : 2;
}

int compare(const void* vp1, const void* vp2) {
	ColoredVertex* v1 = (ColoredVertex*)vp1;
	ColoredVertex* v2 = (ColoredVertex*)vp2;

	// Find orientation
	int orient = orientation(v0.position, v1->position, v2->position);
	if (orient == 0)
		return (distSq(v0.position, v2->position) >= distSq(v0.position, v1->position)) ? -1 : 1;

	return (orient == 2) ? -1 : 1;
}

// Graham Scan for generating convex hull (and avoid concave mesh shapes)
// reference code: https://www.geeksforgeeks.org/convex-hull-using-graham-scan/
// changed so graham scan works on float values: orientation function now uses threshold instead of absolute 0 value
std::vector<ColoredVertex> convexHull(std::vector<ColoredVertex> vertices, int n) {
	std::vector<ColoredVertex> res = std::vector<ColoredVertex>();
	float ymin = vertices[0].position.y;
	int indexMin = 0;
	for (int i = 1; i < n; i++) {
		// find bottom most point
		float y = vertices[i].position.y;
		if ((y < ymin) || ymin == y && vertices[i].position.x < vertices[indexMin].position.x) {
			ymin = vertices[i].position.y;
			indexMin = i;
		}
	}

	// put bottom-most point at first position
	swap(vertices[0], vertices[indexMin]);
	v0 = vertices[0];
	int n_minus = n - 1;
	qsort(&vertices[1], n_minus, sizeof(ColoredVertex), compare);
	// if two or more points make the same angle with p0, remove all but the farthest from p0
	int m = 1;
	for (int i = 1; i < n; i++) {
		// skip i_th vertice while angle of i and i+1 is same with respect to p0
		int j = i + 1;
		while (i < n - 1 && orientation(v0.position, vertices[i].position, vertices[j].position) == 0) {
			i++;
		}
		vertices[m] = vertices[i];
		m++; // update size of modified array
	}
	if (m < 3) return std::vector<ColoredVertex>(); // return empty if less than 3 vertices -- not an object

	std::stack<ColoredVertex> S;
	S.push(vertices[0]);
	S.push(vertices[1]);
	S.push(vertices[2]);

	// process remaining points
	for (int i = 3; i < m; i++) {
		// remove top while angle formed by points next to top, top, and res[i] makes a non-left turn
		while (S.size() > 1 && orientation(nextToTop(S).position, S.top().position, vertices[i].position) != 2)
			S.pop();
		S.push(vertices[i]);
	}

	// check output points
	while (!S.empty()) {
		ColoredVertex v = S.top();
		res.push_back(v);
		S.pop();
	}
	return res;
}

void normalizeProj(vec2& proj) {
	float magnitude = sqrt(proj.x * proj.x + proj.y * proj.y);
	if (magnitude > 1e-9) {
		proj.x /= magnitude;
		proj.y /= magnitude;
	}
}

bool SATcollisionHelper(std::vector<vec2> trsPositions, const Motion& motion2) {
	float bot = motion2.position.y + abs(motion2.scale.y) / 2.f;
	float top = motion2.position.y - abs(motion2.scale.y) / 2.f;
	float right = motion2.position.x + abs(motion2.scale.x) / 2.f;
	float left = motion2.position.x - abs(motion2.scale.x) / 2.f;
	for (int s = 0; s < 2; s++) {
		if (s == 0) {
			// get projections from mesh
			for (int i = 0; i < trsPositions.size(); i++) {
				int j = i + 1;
				j = j % trsPositions.size();
				vec2 proj = { -(trsPositions[j].y - trsPositions[i].y), trsPositions[j].x - trsPositions[i].x };
				normalizeProj(proj);
				float min_r1 = INFINITY, max_r1 = -INFINITY;
				for (int p = 0; p < trsPositions.size(); p++) {
					float val = (trsPositions[p].x * proj.x + trsPositions[p].y * proj.y);
					min_r1 = min(min_r1, val);
					max_r1 = max(max_r1, val);
				}

				float min_r2 = INFINITY, max_r2 = -INFINITY;

				float topLeft = top * proj.y + left * proj.x;
				float topRight = top * proj.y + right * proj.x;
				float botLeft = bot * proj.y + left * proj.x;
				float botRight = bot * proj.y + right * proj.x;
				min_r2 = min(min_r2, topLeft);
				min_r2 = min(min_r2, topRight);
				min_r2 = min(min_r2, botLeft);
				min_r2 = min(min_r2, botRight);

				max_r2 = max(max_r2, topLeft);
				max_r2 = max(max_r2, topRight);
				max_r2 = max(max_r2, botLeft);
				max_r2 = max(max_r2, botRight);

				if (!(max_r2 >= min_r1 && max_r1 >= min_r2)) {
					return false;
				}
			}
		}
		// get projections from rectangle
		else {
			// case 1: projection onto x-axis
			for (int i = 0; i < 2; i++) {
				vec2 proj = { 1, 0 };

				// case 2: projection onto y-axis
				if (i == 1) {
					proj = { 0, 1 };
				}

				float min_r1 = FLT_MAX, max_r1 = FLT_MIN;
				for (int p = 0; p < trsPositions.size(); p++) {
					float val = (trsPositions[p].x * proj.x + trsPositions[p].y * proj.y);
					min_r1 = min(min_r1, val);
					max_r1 = max(max_r1, val);
				}

				float min_r2 = FLT_MAX, max_r2 = FLT_MIN;

				// do top left
				float topLeft = top * proj.y + left * proj.x;
				float topRight = top * proj.y + right * proj.x;
				float botLeft = bot * proj.y + left * proj.x;
				float botRight = bot * proj.y + right * proj.x;
				min_r2 = min(min_r2, topLeft);
				min_r2 = min(min_r2, topRight);
				min_r2 = min(min_r2, botLeft);
				min_r2 = min(min_r2, botRight);

				max_r2 = max(max_r2, topLeft);
				max_r2 = max(max_r2, topRight);
				max_r2 = max(max_r2, botLeft);
				max_r2 = max(max_r2, botRight);


				if (!(max_r2 >= min_r1 && max_r1 >= min_r2)) {
					return false;
				}
			}
		}
	}
	return true;
}

// Separating Axis Theorem between mesh and motion entity
// modified so that the vertice positions would be translated, rotated, and scaled beforehand
bool CollisionSystem::SATcollision(const Mesh* mesh, const Motion& motion1, const Motion& motion2) {
	std::vector<ColoredVertex> vertices = convexHull(mesh->vertices, mesh->vertices.size());
	std::vector<vec2> trsPositions = std::vector<vec2>();
	// modify positions so they're in the right spots
	for (int i = 0; i < vertices.size(); i++) {
		trsPositions.push_back(translateRotateScale(vertices[i].position, motion1));
	}
	return SATcollisionHelper(trsPositions, motion2);
}

bool boxCollision(vec4 box1, vec4 box2) {
	// order of elements is: {min_x, min_y, max_x, max_y}	
	bool y_val = (box1[1] < box2[3]) && (box2[1] < box1[3]);
	bool x_val = (box1[0] < box2[2]) && (box2[0] < box1[2]);
	return y_val && x_val;
}


bool CollisionSystem::rectangleCollides(const Motion& motion1, const Motion& motion2) {
	bool y_val = (motion1.position[1] - abs(motion1.scale.y) / 2.f) < (motion2.position[1] + abs(motion2.scale.y) / 2.f) &&
		(motion2.position[1] - abs(motion2.scale.y) / 2.f) < (motion1.position[1] + abs(motion1.scale.y) / 2.f);
	bool x_val = (motion1.position[0] - abs(motion1.scale.x / 2.f)) < (motion2.position[0] + abs(motion2.scale.x) / 2.f) &&
		(motion2.position[0] - abs(motion2.scale.x / 2.f) < (motion1.position[0] + abs(motion1.scale.x) / 2.f));
	return y_val && x_val;
}

bool lineCollidesDeprecated(const Entity& line,
	       	float min_x, float min_y,
	        float max_x, float max_y) {
	// Takes 4 bounding box inputs to check against a particular line
	DrawnLine& dline = registry.drawnLines.get(line);
	const Motion& line_m = registry.motions.get(line);

	// Determine whether there is intersection
	// const bool y_intersects = !(dline.y_bounds[1] < min_y || dline.y_bounds[0] > max_y);
	// const bool x_intersects = !(dline.x_bounds[1] < min_x || dline.x_bounds[0] > max_x);

	// if (!y_intersects || !x_intersects)
	// 	return false;

	// // Check if line eqn. output is within the bounding box for overlapping x values
	// vec2 x_overlap(std::max(min_x, dline.x_bounds[0]),  std::min(max_x, dline.x_bounds[1]));
	// vec2 test_y = dline.slope * x_overlap + dline.intercept;
	// // Same for x against y values; we check both to guard against perfect vert/horizontal lines
	// vec2 y_overlap(std::max(min_y, dline.y_bounds[0]),  std::min(max_y, dline.y_bounds[1]));
	// vec2 test_x = (1 / dline.slope) * (y_overlap - dline.intercept);

	// const bool result = (test_y[0] >= min_y && test_y[1] <= max_y) ||
	// 		    (test_x[0] >= min_x && test_x[1] <= max_x);

	// apply "undo" rotation to both line and bbox to test collision like a normal upright rect
	Transform transform;
	transform.rotate(-line_m.angle);
	vec2 upper_left(min_x, min_y);
	vec2 lower_right(max_x, max_y);
	upper_left = vec2(transform.mat * vec3(upper_left, 1.f));
	lower_right = vec2(transform.mat * vec3(lower_right, 1.f));
	// need to re-sort because rotation may have changed relative bounds
	vec2 x_bounds  = (upper_left.x < lower_right.x) ? vec2(upper_left.x, lower_right.x) : vec2(lower_right.x, upper_left.x);
	vec2 y_bounds  = (upper_left.y < lower_right.y) ? vec2(upper_left.y, lower_right.y) : vec2(lower_right.y, upper_left.y);
	vec4 test_box(x_bounds[0], y_bounds[0], x_bounds[1], y_bounds[1]);
		
	vec2 test_line = vec2(transform.mat * vec3(line_m.position, 1.f));
	const float half_w = abs(line_m.scale.x) / 2.f;
	const float half_h = abs(line_m.scale.y) / 2.f;
	vec4 line_box(test_line.x - half_w, test_line.y - half_h,
		      test_line.x + half_w, test_line.y + half_h);
	bool result = boxCollision(test_box, line_box);

	// DEBUG code
	if (result && debugging.in_debug_mode) { 
		auto& r = registry.renderRequests.get(line);
		r.used_geometry = GEOMETRY_BUFFER_ID::DEBUG_LINE; // turn the line red
	}
	return result;
}

bool CollisionSystem::lineCollides(const Entity& line,
	float min_x, float min_y,
	float max_x, float max_y) {
	Entity player = registry.players.entities[0];
	Motion &pmotion = registry.motions.get(player);

	const Motion& lmotion = registry.motions.get(line);
	// translate to origin -- set position to 0 basically, so it's just scale / 2;
	float half_w = abs(lmotion.scale.x) / 2.f;
	float half_h = abs(lmotion.scale.y) / 2.f;;
	vec2 topLeft = vec2(-half_w, -half_h);
	vec2 topRight = vec2(half_w, -half_h);
	vec2 botRight = vec2(half_w, half_h);
	vec2 botLeft = vec2(-half_w, half_h);

	std::vector<vec2> trsPositions = { topLeft, topRight, botRight, botLeft };
	// rotate + retranslate point
	for (int i = 0; i < trsPositions.size(); i++) {
		trsPositions[i] = { trsPositions[i].x * cosf(lmotion.angle) - trsPositions[i].y * sinf(lmotion.angle) + lmotion.position.x, trsPositions[i].x * sinf(lmotion.angle) + trsPositions[i].y * cosf(lmotion.angle) + lmotion.position.y };
	}

	bool res = SATcollisionHelper(trsPositions, pmotion);
	//if (res == false) {
	//	printf("collision miss\n");
	//	for (int i = 0; i < trsPositions.size(); i++) {
	//		printf("(%f, %f)\n", trsPositions[i].x, trsPositions[i].y);
	//	}
	//	// recalculate for pmotion
	//	half_w = abs(pmotion.scale.x) / 2.f;
	//	half_h = abs(pmotion.scale.y) / 2.f;
	//	vec2 topLeft = pmotion.position + vec2(-half_w, -half_h);
	//	vec2 topRight = pmotion.position + vec2(half_w, -half_h);
	//	vec2 botRight = pmotion.position + vec2(half_w, half_h);
	//	vec2 botLeft = pmotion.position + vec2(-half_w, half_h);

	//	trsPositions = { topLeft, topRight, botRight, botLeft };
	//	for (int i = 0; i < trsPositions.size(); i++) {
	//		printf("(%f, %f)\n", trsPositions[i].x, trsPositions[i].y);
	//	}
	//}
	return res;
}
