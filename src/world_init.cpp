#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"

vec2 translateRotateScale(vec3 position, const Motion& motion) {
	vec2 positions = { position.x, position.y };
	float scaled_x = positions[0] * motion.scale.x;
	float scaled_y = positions[1] * motion.scale.y;
	positions = { scaled_x * cosf(motion.angle) - scaled_y * sinf(motion.angle) + motion.position.x, scaled_x * sinf(motion.angle) + scaled_y * cosf(motion.angle) + motion.position.y };
	return positions;
}

vec4 getBox(const Mesh* mesh, const Motion& motion) {
	float max_x = FLT_MIN;
	float max_y = FLT_MIN;
	float min_x = FLT_MAX;
	float min_y = FLT_MAX;
	for (size_t i = 0; i < mesh->vertices.size(); i++) {
		vec2 positions = translateRotateScale(mesh->vertices[i].position, motion);

		max_x = max(max_x, positions[0]);
		max_y = max(max_y, positions[1]);
		min_x = min(min_x, positions[0]);
		min_y = min(min_y, positions[1]);
	}

	vec4 box = { max_x, max_y, min_x, min_y };
	return box;
}

Entity createOliver(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::OLIVER);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = {40.f, 80.f};
	motion.gravityScale = 1.f;
	motion.grounded = false;

	registry.players.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::OLIVER,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createPlatform(RenderSystem* renderer, vec2 position, vec2 size)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = size;
	motion.fixed = true;

	registry.platforms.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLATFORM,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createWall(RenderSystem* renderer, vec2 position, vec2 size)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = size;
	motion.fixed = true;

	registry.walls.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::EARTH,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createLine(vec2 position, vec2 scale, float angle)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::EGG,
		 GEOMETRY_BUFFER_ID::DEBUG_LINE });

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = angle;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = scale;
	motion.fixed = true;

	registry.toDrawOns.emplace(entity);

	//registry.debugComponents.emplace(entity);
	return entity;
}

Entity createBoulder(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 100.f };
	motion.position = position;
	motion.gravityScale = 0.05f;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ -BOULDER_BB_WIDTH, BOULDER_BB_HEIGHT });

	registry.deadlys.emplace(entity);
	registry.boulders.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BOULDER,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createChaseBoulder(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.gravityScale = 0.f;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ -BOULDER_BB_WIDTH / 1.3, BOULDER_BB_HEIGHT / 1.3 });

	registry.deadlys.emplace(entity);
	registry.advancedAIs.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::CHASEBOULDER,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createPencil(RenderSystem* renderer, vec2 position, vec2 size)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the pencil
	auto& motion = registry.motions.emplace(entity);
	motion.position = position;

	motion.scale = size;
	motion.fixed = true;

	// Create a RenderRequest for the pencil
	registry.pencil.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PENCIL, 
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createCheckpoint(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);


	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = {70.f, 100.f};
	motion.fixed = true;

	// Create a RenderRequest for the checkpoint flag
	registry.checkpoints.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::CHECKPOINT,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createEndpoint(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);


	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = { 70.f, 100.f };
	motion.fixed = true;

	// Create a RenderRequest for the checkpoint flag
	registry.levelEnds.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::LEVELEND,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createBackground(RenderSystem* renderer)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);


	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = {window_width_px/ 2, window_height_px / 2};
	motion.scale = { window_width_px - 10, window_height_px - 10 };
	motion.fixed = true;

	// Create a RenderRequest for the background
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BACKGROUND,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createPaintCan(RenderSystem* renderer, vec2 position, vec2 size, float paintRefill, bool fixed)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 200.f, 100.f };
	motion.position = position;
	motion.gravityScale = 12.f;
	motion.scale = size;
	motion.fixed = fixed;

	registry.eatables.emplace(entity);
	PaintCan& p = registry.paintCans.emplace(entity);
	p.paintRefill = paintRefill;
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PAINTCAN,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}


Entity createTutorialDraw(RenderSystem* renderer) {
	Entity e = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(e, &mesh);

	auto& motion = registry.motions.emplace(e);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = {window_width_px / 2, window_height_px / 4};
	motion.scale = { window_width_px * 0.1, window_height_px * 0.2 };
	motion.fixed = true;

	// Create a RenderRequest for the tutorial
	registry.renderRequests.insert(
		e, { TEXTURE_ASSET_ID::TUTORIALDRAW,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE });

	return e;
}

Entity createTutorialJump(RenderSystem* renderer) {
	Entity e = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(e, &mesh);

	auto& motion = registry.motions.emplace(e);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = { window_width_px / 2, window_height_px / 4 - 100 };
	motion.scale = { window_width_px * 0.3, window_height_px * 0.4 };
	motion.fixed = true;

	// Create a RenderRequest for the tutorial
	registry.renderRequests.insert(
		e, { TEXTURE_ASSET_ID::TUTORIALJUMP,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE });

	return e;
}

Entity createTutorialMainMenu(RenderSystem* renderer) {
	Entity e = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(e, &mesh);

	auto& motion = registry.motions.emplace(e);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = { window_width_px / 2, window_height_px / 4 };
	motion.scale = { window_width_px * 0.1, window_height_px * 0.2 };
	motion.fixed = true;

	// Create a RenderRequest for the tutorial
	registry.renderRequests.insert(
		e, { TEXTURE_ASSET_ID::TUTORIALMAINMENU,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE });

	return e;
}

Entity createTutorialMove(RenderSystem* renderer) {
	Entity e = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(e, &mesh);

	auto& motion = registry.motions.emplace(e);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = { window_width_px / 2, window_height_px / 4 };
	motion.scale = { window_width_px * 0.3, window_height_px * 0.4 };
	motion.fixed = true;

	// Create a RenderRequest for the tutorial
	registry.renderRequests.insert(
		e, { TEXTURE_ASSET_ID::TUTORIALMOVE,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE });

	return e;
}

Entity createTutorialRestart(RenderSystem* renderer) {
	Entity e = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(e, &mesh);

	auto& motion = registry.motions.emplace(e);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = { window_width_px / 2, window_height_px / 4 };
	motion.scale = { window_width_px * 0.3, window_height_px * 0.4 };
	motion.fixed = true;

	// Create a RenderRequest for the tutorial
	registry.renderRequests.insert(
		e, { TEXTURE_ASSET_ID::TUTORIALRESTART,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE });

	return e;
}

Entity createSpikes(RenderSystem* renderer, vec2 position, vec2 size, float radian)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion component
	auto& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = radian;
	motion.scale = size;
	motion.fixed = true;
	motion.grounded = true;

	// Create a RenderRequest for the spikes
	registry.deadlys.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::SPIKES,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createArcher(RenderSystem * renderer, vec2 position, vec2 size)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	auto & motion = registry.motions.emplace(entity);
	auto& coolDownTimer = registry.arrowCooldowns.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.scale = size;
	registry.archers.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::GREENENEMY,
				 EFFECT_ASSET_ID::TEXTURED,
				 GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createHint(RenderSystem* renderer, vec2 position, std::string text, vec2 textPos)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);


	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = { 47.f, 60.f };
	motion.fixed = true;

	Hint& h = registry.hints.emplace(entity);
	h.text = text;
	h.textPos = textPos;
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::HINT1,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createMainMenu(RenderSystem* renderer, vec2 position, vec2 size)
{
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = size;
	motion.fixed = true;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::MAINMENU,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}