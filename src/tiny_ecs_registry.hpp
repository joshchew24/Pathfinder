#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
	// Callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	// Manually created list of all components this game has
	// TODO: A1 add a LightUp component
	ComponentContainer<DeathTimer> deathTimers;
	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Player> players;
	ComponentContainer<Mesh*> meshPtrs;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<Eatable> eatables;
	ComponentContainer<Deadly> deadlys;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<vec3> colors;
	ComponentContainer<Platform> platforms;
	ComponentContainer<Pencil> pencil;
	ComponentContainer<Checkpoint> checkpoints;
	ComponentContainer<Wall> walls;
	ComponentContainer<Drawing> drawings;
	ComponentContainer<DrawnLine> drawnLines;
	ComponentContainer<DrawnJoint> drawnJoints;
	ComponentContainer<DrawnPoint> drawnPoints;
	ComponentContainer<advancedAI> advancedAIs;
	ComponentContainer<levelEnd> levelEnds;
	ComponentContainer<Boulder> boulders;
	ComponentContainer<PaintCan> paintCans;
	ComponentContainer<BezierProjectile> projectiles;
	ComponentContainer<Archer> archers;
	ComponentContainer<ArrowCooldown> arrowCooldowns;
	ComponentContainer<Particle> particles;
	ComponentContainer<ParticleEmitter> particleEmitters;
	ComponentContainer<toDrawOn> toDrawOns;
	ComponentContainer<Hint> hints;

	// constructor that adds all containers for looping over them
	// IMPORTANT: Don't forget to add any newly added containers!
	ECSRegistry()
	{
		// TODO: A1 add a LightUp component
		registry_list.push_back(&deathTimers);
		registry_list.push_back(&motions);
		registry_list.push_back(&collisions);
		registry_list.push_back(&players);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&screenStates);
		registry_list.push_back(&eatables);
		registry_list.push_back(&deadlys);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&colors);
		registry_list.push_back(&platforms);
		registry_list.push_back(&pencil);
		registry_list.push_back(&checkpoints);
		registry_list.push_back(&walls);
		registry_list.push_back(&drawings);
		registry_list.push_back(&drawnLines);
		registry_list.push_back(&drawnPoints);
		registry_list.push_back(&advancedAIs);
		registry_list.push_back(&levelEnds);
		registry_list.push_back(&boulders);
		registry_list.push_back(&paintCans);
		registry_list.push_back(&projectiles);
		registry_list.push_back(&archers);
		registry_list.push_back(&particles);
		registry_list.push_back(&particleEmitters);
		registry_list.push_back(&toDrawOns);
		registry_list.push_back(&hints);
	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}
};

extern ECSRegistry registry;
