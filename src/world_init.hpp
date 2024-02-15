#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

const float BOULDER_BB_WIDTH = 0.3f * 300.f;
const float BOULDER_BB_HEIGHT = 0.3f * 202.f;

// These are ahrd coded to the dimensions of the entity texture
// the player
Entity createOliver(RenderSystem* renderer, vec2 pos);

Entity createPlatform(RenderSystem* renderer, vec2 pos, vec2 size);
// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);

Entity createBoulder(RenderSystem* renderer, vec2 position);


