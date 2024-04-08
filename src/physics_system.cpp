// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>
#include <stack>
#include <cfloat>

//vec2 calculateControlPoint(const glm::vec2& start, const glm::vec2& end, float height) {
//	glm::vec2 midPoint = (start + end) / 2.0f;
//	midPoint.y -= height; // Adjust the height to control the arc's steepness
//	return midPoint;
//}

vec2 quadraticBezier(const glm::vec2& start, const glm::vec2& control, const glm::vec2& end, float t) {
	float u = 1 - t;
	float tt = t * t;
	float uu = u * u;

	glm::vec2 point = uu * start; // First term
	point += 2 * u * t * control; // Second term
	point += tt * end; // Third term
	return point;
}

void PhysicsSystem::step(float elapsed_ms)
{
	//elapsed_ms = clamp(elapsed_ms, 0.f, 8.f);
	auto& motion_registry = registry.motions;
	for (uint i = 0; i < motion_registry.size(); i++)
	{
		Motion& motion = motion_registry.components[i];
		if (motion.fixed) {
			continue;
		}
		float step_seconds = elapsed_ms / 1000.f;
		Entity entity = motion_registry.entities[i];

		// decide y accel and vel
		if (motion.isJumping) {
			if (motion.timeJumping <= 150.f) {
				motion.grounded = false;
				motion.velocity.y = -jump_height;
				motion.acceleration.y = 0;
				motion.timeJumping += elapsed_ms;
			}
			else {
				motion.isJumping = false;
			}
		}
		else if (motion.grounded) {
			motion.jumpsLeft = 1;
			motion.acceleration.y = 0.f;
			motion.velocity.y = 0.f;
		}
		else {
			motion.acceleration.y = gravity * motion.gravityScale;
			motion.velocity.y = clamp(motion.velocity.y + motion.acceleration.y * step_seconds, -terminal_velocity, terminal_velocity);
		}
    
		if (registry.players.has(entity)) {
			checkWindowBoundary(motion);
			applyFriction(motion, step_seconds);
			if (registry.deathTimers.has(entity)) {
				motion.velocity.x = 0.f;
			}
		}

		updateGroundedStateForEntities(registry, registry.paintCans, [](Motion& motion, bool isGrounded) {
			motion.grounded = isGrounded;
			});
		updateGroundedStateForEntities(registry, registry.archers, [](Motion& motion, bool isGrounded) {
			motion.grounded = isGrounded;
			});
      
		motion.last_position = motion.position;
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

			if (collisionSystem.collides(motion_i, entity_i, motion_j, entity_j)) {
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}
	}

	// bezier movement
	ComponentContainer<BezierProjectile>& projectile_container = registry.projectiles;
	ComponentContainer<Platform>& platform_container = registry.platforms;
	std::vector<Entity> to_be_removed;
	for (uint i = 0; i < projectile_container.components.size(); i++) {
		BezierProjectile& projectile = projectile_container.components[i];
		Entity projectileEntity = projectile_container.entities[i];
		Motion& projectile_motion = motion_registry.get(projectile_container.entities[i]);
		Motion& player_motion = motion_registry.get(registry.players.entities[0]);
		projectile.elapsedTime += elapsed_ms;
		float t = projectile.elapsedTime / 2000;
		if (t > 1.0) t = 1.0;
		if (t >= 1.0) {
			to_be_removed.push_back(projectile_container.entities[i]);
		}
		vec2 currentPosition = quadraticBezier(projectile.startPosition, projectile.controlPoint, projectile.targetPosition, t);
		vec2 tangentVec = 2.0f * (1.0f - t) * (projectile.controlPoint - projectile.startPosition) + 2.0f * t * (projectile.targetPosition - projectile.controlPoint);
		float radian = atan(-tangentVec.y, tangentVec.x);
		projectile_motion.angle = radian;
		projectile_motion.position = currentPosition;
	}

	for (Entity e : to_be_removed) {
		registry.remove_all_components_of(e);
	}

	// ComponentContainer<Platform>& platform_container = registry.platforms;
	//ComponentContainer<Motion>& motion_container = registry.motions;
	Motion& player = motion_container.get(registry.players.entities[0]);
	bool touching_any_platform = false;
	for (uint i = 0; i < platform_container.components.size(); i++) {
		Motion& platform = motion_container.get(platform_container.entities[i]);
		if (collisionSystem.rectangleCollides(platform, player)) {
			touching_any_platform = true;
			break;
		}
	}

	// Check collisions with drawn lines
	Entity& oliver = registry.players.entities[0];
	Mesh *meshptr = registry.meshPtrs.get(oliver);
	vec4 bbox = getBox(meshptr, player);
	//printf("min x = %f, max x = %f\n", bbox[2], bbox[0]);
	for (auto &line_ent : registry.drawnLines.entities) {
		const DrawnLine& line = registry.drawnLines.get(line_ent);
		if (collisionSystem.lineCollides(line_ent, bbox[2], bbox[3], bbox[0], bbox[1])) {
			registry.collisions.emplace_with_duplicates(oliver, line_ent);
			registry.collisions.emplace_with_duplicates(line_ent, oliver);
			//player.position = player.last_position;
			const DrawnLine& l = registry.drawnLines.get(line_ent);
			const Motion& lm = registry.motions.get(line_ent);
			// if player is above line, set player to grounded
			// if the player is on the edge of the line, use the edge of the line's x value instead
			float player_pos = player.position.x;
			float min_x = min(line.x_bounds[0], line.x_bounds[1]);
			float max_x = max(line.x_bounds[0], line.x_bounds[1]);
			if (player.position.x < min_x) {
				player_pos = min_x;
			}
			else if (player.position.x > max_x) {
				player_pos = max_x;
			}
			float line_y_pos = l.slope * (player_pos - lm.position.x) + lm.position.y;
			if (player.position.y <= line_y_pos) {
				touching_any_platform = true;
				break;
			}
		}
	}
	player.grounded = touching_any_platform;
}

template<typename T>
void PhysicsSystem::updateGroundedStateForEntities(ECSRegistry& registry, const ComponentContainer<T>& entityContainer, UpdateGroundedStateFunc updateFunc) {
	auto& platformContainer = registry.platforms;

	for (auto& entity : entityContainer.entities) {
		Motion& entityMotion = registry.motions.get(entity);
		bool isTouchingPlatform = false;

		for (auto& platformEntity : platformContainer.entities) {
			Motion& platformMotion = registry.motions.get(platformEntity);

			if (collisionSystem.rectangleCollides(entityMotion, platformMotion)) {
				isTouchingPlatform = true;
				entityMotion.grounded = true;
				entityMotion.velocity.y = 0;
				float platformTop = platformMotion.position.y + platformMotion.scale.y / 2;
				entityMotion.position.y = platformTop - entityMotion.scale.y / 2;
				break;
			}
		}


		// Call the provided function to handle the update
		updateFunc(entityMotion, isTouchingPlatform);
	}
}

// used to keep player within window boundary
// if entity hits top, left, or right, change velocity so the entity bounces off boundary
void PhysicsSystem::checkWindowBoundary(Motion& motion) {
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
}

void PhysicsSystem::applyFriction(Motion& motion, float elapsed_ms) {
	float step_seconds = elapsed_ms / 1000.f;
	// apply horizontal friction to player
	if (motion.velocity.x != 0.0 && motion.acceleration.x != 0.0) {
		float delta_vel = motion.acceleration.x * step_seconds;
		// this conditional ASSUMES we are decelerating due to friction, and should stop at 0
		if (abs(motion.velocity.x) < abs(delta_vel)) {
			motion.velocity.x = 0.0;
		}
		else {
			motion.velocity.x = clamp(motion.velocity.x + delta_vel, -terminal_velocity, terminal_velocity);
		}
	}
	else {
		motion.acceleration.x = 0.0f;
	}
}
