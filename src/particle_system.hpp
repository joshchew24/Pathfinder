
#pragma once
class ParticleSystem
{
public:
	void init() {

	}
	void step(float elapsed_ms);
	void spawn_particle(const ParticleEmitter& emitter);
};