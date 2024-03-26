// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>
#include <stack>
#include <cfloat>

vec2 calculateControlPoint(const glm::vec2& start, const glm::vec2& end, float height) {
	glm::vec2 midPoint = (start + end) / 2.0f;
	midPoint.y -= height; // Adjust the height to control the arc's steepness
	return midPoint;
}

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
			printf("%f\n", motion.timeJumping);
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
		else if (registry.boulders.has(entity)) {
			// this should just be set once in the boulder creation
			motion.acceleration.y = gravity / 20;
			motion.velocity.y = clamp(motion.velocity.y + motion.acceleration.y, -terminal_velocity, terminal_velocity);
		}
		else if (!motion.notAffectedByGravity) {
			motion.acceleration.y = gravity;
			motion.velocity.y = clamp(motion.velocity.y + motion.acceleration.y, -terminal_velocity, terminal_velocity);
		}
    
		if (registry.players.has(entity)) {
			checkWindowBoundary(motion);
			applyFriction(motion);
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

	ComponentContainer<BezierProjectile>& projectile_container = registry.projectiles;
	ComponentContainer<Platform>& platform_container = registry.platforms;
	for (uint i = 0; i < projectile_container.components.size(); i++) {
		BezierProjectile& projectile = projectile_container.components[i];
		Entity projectileEntity = projectile_container.entities[i];
		Motion& projectile_motion = motion_registry.get(projectile_container.entities[i]);
		Motion& player_motion = motion_registry.get(registry.players.entities[0]);
		projectile.elapsedTime += elapsed_ms;
		float t = elapsed_ms / 1500;
		std::printf("elapsed_ms: %f, Elapsed Time: %f, t: %f\n",
			elapsed_ms, projectile.flightDuration, t);
		if (t > 1.0) t = 1.0;
		if (t >= 1.0) {
			registry.remove_all_components_of(projectileEntity);
		}
		vec2 startPosition = projectile_motion.position;
		glm::vec2 endPosition = projectile.targetPosition;
		float distance = glm::distance(startPosition, endPosition);
		float arcHeight = distance * 0.5;
		vec2 controlPoint = calculateControlPoint(startPosition, endPosition, arcHeight);
		vec2 currentPosition = quadraticBezier(startPosition, controlPoint, endPosition, t);
		vec2 tangentVec = 2.0f * (1.0f - t) * (controlPoint - startPosition) + 2.0f * t * (endPosition - controlPoint);
		float radian = atan(-tangentVec.y, tangentVec.x);
		projectile_motion.angle = radian;
		projectile_motion.position = currentPosition;
		if (projectile_motion.velocity.y == 0) {
			registry.remove_all_components_of(projectileEntity);
		}
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

void PhysicsSystem::applyFriction(Motion& motion) {
	// apply horizontal friction to player
	if (motion.velocity.x != 0.0 && motion.acceleration.x != 0.0) {
		// this conditional ASSUMES we are decelerating due to friction, and should stop at 0
		if (abs(motion.velocity.x) < abs(motion.acceleration.x)) {
			motion.velocity.x = 0.0;
		}
		else {
			motion.velocity.x = clamp(motion.velocity.x + motion.acceleration.x, -terminal_velocity, terminal_velocity);
		}
	}
	else {
		motion.acceleration.x = 0.0f;
	}
}