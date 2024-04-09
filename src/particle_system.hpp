
#pragma once
class ParticleSystem
{

	float frameCount = 0;
	float frameMax = 100;
public:
	void init() {

	}
	void step(float elapsed_ms);
	void spawn_particle(const ParticleEmitter& emitter, Motion m);
};