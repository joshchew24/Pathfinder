
#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include <particle_system.hpp>
#include <random>

void ParticleSystem::step(float elapsed_ms) {
	// Update particle positions
	float step_second = elapsed_ms / 1000.f;

	auto& particles_container = registry.particles;
	std::vector<Entity> to_be_removed;

	for (auto& particleEntity : particles_container.entities) {
		auto& particle = particles_container.get(particleEntity);
		auto& motion = registry.motions.get(particleEntity);

		motion.position += motion.velocity * step_second;
		particle.life -= step_second;

		if (particle.life <= 0.0f) {
			to_be_removed.push_back(particleEntity);
		}
	}

	for (auto& particle : to_be_removed) {
		registry.remove_all_components_of(particle);
	}

	auto& emtitter_container = registry.particleEmitters;
	//printf("size of emitter: %d, size of particles: %d\n", emtitter_container.size(), particles_container.size());

	frameCount += elapsed_ms;

	if (frameCount >= frameMax) {
		for (auto& emitterEntity : emtitter_container.entities) {
			auto& emitter = emtitter_container.get(emitterEntity);
			int particlesToSpawn = emitter.particles_per_second;
			//printf("particles to spawn: %d\n", particlesToSpawn);
			if (registry.pencil.has(emitterEntity) && !drawings.currently_drawing()) {
				continue;
			}
			for (int i = 0; i < particlesToSpawn; i++) {
				spawn_particle(emitter, registry.motions.get(emitterEntity));
			}
		}
		frameCount = 0;
	}

}

void ParticleSystem::spawn_particle(const ParticleEmitter& emitter, Motion m) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis_offset_x(-10.0, 10.0);
	std::uniform_real_distribution<> dis_offset_y(-2.0, 2.0);
	std::uniform_real_distribution<> dis_velocity(-10.0, 10.0);

	Entity entity = Entity();
	auto& particle = registry.particles.emplace(entity);
	particle.color = emitter.color;
	particle.life = emitter.lifespan;
	auto& motion = registry.motions.emplace(entity);

	motion.position = {
		m.position.x + dis_offset_x(gen),
		m.position.y + dis_offset_y(gen)
	};

	motion.velocity = {
		m.velocity.x + dis_velocity(gen),
		m.velocity.y + dis_velocity(gen)
	};

	motion.scale = { 10, 10 };
	motion.grounded = true;
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::CIRCLEPARTICLE,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE });
}
