#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

const float BOULDER_BB_WIDTH = 0.3f * 300.f;
const float BOULDER_BB_HEIGHT = 0.3f * 300.f;

// used to translate, rotate, and scale the player
vec2 translateRotateScale(vec3 position, const Motion& motion);

// These are ahrd coded to the dimensions of the entity texture
// the player
Entity createOliver(RenderSystem* renderer, vec2 pos);

Entity createPlatform(RenderSystem* renderer, vec2 pos, vec2 size);
// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);

Entity createBoulder(RenderSystem* renderer, vec2 position);

Entity createChaseBoulder(RenderSystem* renderer, vec2 position);

Entity createPencil(RenderSystem* renderer, vec2 pos, vec2 size);

Entity createCheckpoint(RenderSystem* renderer, vec2 pos);

Entity createWall(RenderSystem* renderer, vec2 position, vec2 size);

Entity createEndpoint(RenderSystem* renderer, vec2 position);

Entity createBackground(RenderSystem* renderer);

Entity createPaintCan(RenderSystem* renderer, vec2 pos, vec2 size);

Entity createTutorial(RenderSystem* renderer);

Entity createSpikes(RenderSystem* renderer, vec2 pos, vec2 size);