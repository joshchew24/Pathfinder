#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are ahrd coded to the dimensions of the entity texture
// the player
Entity createOliver(RenderSystem* renderer, vec2 pos);

Entity createPlatform(RenderSystem* renderer, vec2 pos, vec2 size);
// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);


