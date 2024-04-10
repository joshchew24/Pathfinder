
#pragma once
#include <drawing_system.hpp>
class ParticleSystem
{

	float frameCount = 0;
	float frameMax = 75;
public:
	void init() {

	}
	void step(float elapsed_ms);
	void spawn_particle(const ParticleEmitter& emitter, Motion m);
};