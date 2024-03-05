// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>
#include <stack>

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection
bool collides(const Motion& motion1, const Motion& motion2)
{
	vec2 dp = motion1.position - motion2.position;
	float dist_squared = dot(dp,dp);
	const vec2 other_bonding_box = get_bounding_box(motion1) / 2.f;
	const float other_r_squared = dot(other_bonding_box, other_bonding_box);
	const vec2 my_bonding_box = get_bounding_box(motion2) / 2.f;
	const float my_r_squared = dot(my_bonding_box, my_bonding_box);
	const float r_squared = max(other_r_squared, my_r_squared);
	if (dist_squared < r_squared)
		return true;
	return false;
}

// global vertex for the sorting portion of convexHull function
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

// Separating Axis Theorem between mesh and motion entity
// modified so that the vertice positions would be translated, rotated, and scaled beforehand
bool SATcollision(const Mesh* mesh, const Motion& motion1, const Motion& motion2) {
	float bot = motion2.position.y + abs(motion2.scale.y) / 2.f;
	float top = motion2.position.y - abs(motion2.scale.y) / 2.f;
	float right = motion2.position.x + abs(motion2.scale.x) / 2.f;
	float left = motion2.position.x - abs(motion2.scale.x) / 2.f;
	std::vector<ColoredVertex> vertices = convexHull(mesh->vertices, mesh->vertices.size());
	std::vector<vec2> trsPositions = std::vector<vec2>();
	// modify positions so they're in the right spots
	for (int i = 0; i < vertices.size(); i++) {
		trsPositions.push_back(translateRotateScale(vertices[i].position, motion1));
	}
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

bool rectangleCollides(const Motion& motion1, const Motion& motion2) {
	bool y_val = (motion1.position[1] - abs(motion1.scale.y)/2.f) < (motion2.position[1] + abs(motion2.scale.y)/2.f) &&
		(motion2.position[1] - abs(motion2.scale.y)/2.f) < (motion1.position[1] + abs(motion1.scale.y)/2.f);
	bool x_val = (motion1.position[0] - abs(motion1.scale.x/2.f)) < (motion2.position[0] + abs(motion2.scale.x)/2.f) &&
		(motion2.position[0] - abs(motion2.scale.x/2.f) < (motion1.position[0] + abs(motion1.scale.x)/2.f));
	return y_val && x_val;
}

void PhysicsSystem::step(float elapsed_ms)
{
	elapsed_ms = clamp(elapsed_ms, 0.f, 8.f);
	auto& motion_registry = registry.motions;
	for (uint i = 0; i < motion_registry.size(); i++)
	{
		float step_seconds = elapsed_ms / 1000.f;
		Motion& motion = motion_registry.components[i];
		Entity entity = motion_registry.entities[i];
		if (motion.fixed) {
			continue;
		}

		//if (motion.onlyGoDown) {
		//	motion.acceleration.x = 0;
		//	motion.velocity.x = 0;
		//	motion.acceleration.y = gravity;
		//	motion.velocity.y = clamp(motion.velocity.y + motion.acceleration.y, -400.f, 400.f);
		//	motion.position += motion.velocity * step_seconds;
		//	motion.onlyGoDown = false;
		//	continue;
		//}
		// apply gravity
		if (motion.grounded) {
			motion.acceleration.y = 0.f;
			motion.velocity.y = 0.f;
		}
		else if (registry.boulders.has(entity)) {
			motion.acceleration.y = gravity/20;
			motion.velocity.y = clamp(motion.velocity.y + motion.acceleration.y, -600.f, 600.f);
		}
		else if(!motion.notAffectedByGravity){
			motion.acceleration.y = gravity;
			motion.velocity.y = clamp(motion.velocity.y + motion.acceleration.y, -600.f, 600.f);
		}
    
		// if player hits top, left, or right, change velocity so the player bounces off boundary
		if (registry.players.has(motion_registry.entities[i])) {
			if (motion.position.x - abs(motion.scale.x) / 2 < 0) {
				motion.position.x += 1.f;
				motion.velocity.x = 0.f;
			}
			else if (motion.position.x + abs(motion.scale.x) / 2 > window_width_px) {
				motion.position.x -= 1.f;
				motion.velocity.x = 0.f;
			}
			else if (motion.position.y - abs(motion.scale.y) / 2 < 0) {
				motion.position.y += 1.f;
				motion.velocity.y = 0.f;
			}
			
			if (motion.velocity.x != 0.0 && motion.acceleration.x != 0.0) {
				// this conditional ASSUMES we are decelerating due to friction, and should stop at 0
				if (abs(motion.velocity.x) < abs(motion.acceleration.x)) {
					motion.velocity.x = 0.0;
				}
				else {
					motion.velocity.x = clamp(motion.velocity.x + motion.acceleration.x, -TERMINAL_VELOCITY, TERMINAL_VELOCITY);
				}
			}
			else {
				motion.acceleration.x = 0.0f;
			}
		}
		updatePaintCanGroundedState();

		motion.position += motion.velocity * step_seconds;
	}

	// Check for collisions between all moving entities
    ComponentContainer<Motion> &motion_container = registry.motions;
	for(uint i = 0; i<motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];
		
		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for(uint j = i+1; j<motion_container.components.size(); j++)
		{
			Motion& motion_j = motion_container.components[j];
			Entity entity_j = motion_container.entities[j];

			// check if entity_i is a mesh for the paint can for SAT mesh collision
			if (registry.renderRequests.has(entity_i) && registry.renderRequests.get(entity_i).used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT
				&& registry.meshPtrs.has(entity_i) && registry.meshPtrs.get(entity_i)->vertices.size() > 0 && SATcollision(registry.meshPtrs.get(entity_i), motion_i, motion_j)) {
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
			// check if entity_j is a mesh for the paint can for SAT mesh collision
			else if (registry.renderRequests.has(entity_j) && registry.renderRequests.get(entity_j).used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT
				&& registry.meshPtrs.has(entity_j) && registry.meshPtrs.get(entity_j)->vertices.size() > 0 && SATcollision(registry.meshPtrs.get(entity_j), motion_j, motion_i)) {
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
			// else if no mesh geometry collision, then have rectangular collision instead
			else if (rectangleCollides(motion_i, motion_j)) {
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}
	}

	ComponentContainer<Platform>& platform_container = registry.platforms;
	//ComponentContainer<Motion>& motion_container = registry.motions;
	Motion& player = motion_container.get(registry.players.entities[0]);
	bool touching_any_platform = false;
	for (uint i = 0; i < platform_container.components.size(); i++) {
		Motion& platform = motion_container.get(platform_container.entities[i]);
		if (rectangleCollides(platform, player)) {
			touching_any_platform = true;
			break;
		}
	}
	player.grounded = touching_any_platform;

}

void PhysicsSystem::updatePaintCanGroundedState() {
	auto& paintCanRegistry = registry.paintCans;
	auto& platformContainer = registry.platforms;

	for (auto& paintCanEntity : paintCanRegistry.entities) {
		Motion& paintCanMotion = registry.motions.get(paintCanEntity);
		bool isTouchingPlatform = false;

		for (auto& platformEntity : platformContainer.entities) {
			Motion& platformMotion = registry.motions.get(platformEntity);

			if (rectangleCollides(paintCanMotion, platformMotion)) {
				isTouchingPlatform = true;
				paintCanMotion.grounded = true;
				paintCanMotion.velocity.y = 0;


				float platformTop = platformMotion.position.y + platformMotion.scale.y / 2.0f;
				paintCanMotion.position.y = platformTop - paintCanMotion.scale.y / 2.0f;
				break; 
			}
		}

		if (!isTouchingPlatform) {
			paintCanMotion.grounded = false;
		}
	}
}