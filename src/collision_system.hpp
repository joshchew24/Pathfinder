#pragma once

#include "common.hpp"
#include "components.hpp"
#include <stack>
#include "tiny_ecs_registry.hpp"


class CollisionSystem
{

public:
	CollisionSystem() {}

	void init() {

	}

	bool collides(const Motion& motion1, const Entity& entity1, const Motion& motion2, const Entity& entity2);

	bool SATcollision(const Mesh* mesh, const Motion& motion1, const Motion& motion2);

	bool rectangleCollides(const Motion& motion1, const Motion& motion2);
};