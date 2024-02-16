// internal
#include "physics_system.hpp"
#include "world_init.hpp"

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

bool rectangleCollides(const Motion& motion1, const Motion& motion2) {
	bool y_val = (motion1.position[1] - abs(motion1.scale.y)/2.f) < (motion2.position[1] + abs(motion2.scale.y)/2.f) &&
		(motion2.position[1] - abs(motion2.scale.y)/2.f) < (motion1.position[1] + abs(motion1.scale.y)/2.f);
	bool x_val = (motion1.position[0] - abs(motion1.scale.x/2.f)) < (motion2.position[0] + abs(motion2.scale.x)/2.f) &&
		(motion2.position[0] - abs(motion2.scale.x/2.f) < (motion1.position[0] + abs(motion1.scale.x)/2.f));
	return y_val && x_val;
}

void PhysicsSystem::step(float elapsed_ms)
{
	auto& motion_registry = registry.motions;
	for (uint i = 0; i < motion_registry.size(); i++)
	{
		Motion& motion = motion_registry.components[i];
		if (motion.fixed) continue;
		float step_seconds = elapsed_ms / 1000.f;
		// apply gravity
		if (motion.grounded) {
			motion.acceleration.y = 0.f;
			motion.velocity.y = 0.f;
		}
		else {
			motion.acceleration.y = 9.f;
			motion.velocity.y += motion.acceleration.y;
		}
    
		// if player hits top, left, or right, change velocity so the player bounces off boundary
		if (registry.players.has(motion_registry.entities[i])) {
			if (motion.position.x - abs(motion.scale.x) / 2 < 0) {
				motion.velocity.x = 250.f;
			}
			else if (motion.position.x + abs(motion.scale.x) / 2 > window_width_px) {
				motion.velocity.x = -250.f;
			}
			else if (motion.position.y - abs(motion.scale.y) / 2 < 0) {
				motion.velocity.y *= -1;
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
			if (rectangleCollides(motion_i, motion_j))
			{
				Entity entity_j = motion_container.entities[j];
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