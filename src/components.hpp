#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"

// Player component
struct Player
{

};

struct Platform
{

};

// tracking the pencil
struct Pencil
{

};

// Top-level drawing abstraction component
struct Drawing
{

};

// Drawn Points
struct DrawnPoint
{
	Entity drawing;
	vec2 position;
};

// Drawn Lines
struct DrawnLine
{
	Entity drawing;
	Entity p1; // first DrawnPoint ent
	Entity p2; // second DrawnPoint ent
	
	vec2 x_bounds;
	vec2 y_bounds;
	vec2 collide_dir;

	// y = mx + b linear eqn coeffs
	float slope;
	float intercept;
	float line_width = 10.f;

	bool perp_collision = false;
};

// Drawn Line Joints
struct DrawnJoint
{
	Entity drawing;
	Entity l1; // line 1
	Entity l2;
};

// Eagles have a hard shell
struct Deadly
{

};

// Bug and Chicken have a soft shell
struct Eatable
{

};

struct Checkpoint
{

};

struct Wall
{

};

struct advancedAI
{

};

struct Boulder
{

};

struct levelEnd 
{

};

struct PaintCan
{

};

// All data relevant to the shape and motion of entities
struct Motion {
	vec2 position = { 0.f, 0.f };
	vec2 last_position = { 0.f, 0.f};
	float angle = 0.f;
	vec2 velocity = { 0.f, 0.f };
	vec2 scale = { 10.f, 10.f };
	vec2 acceleration = { 0.f, 0.f };
	float gravityScale = 1.0f;
	bool grounded = false;
	bool fixed = false;
	bool isJumping = false;
	float timeJumping = 0.f;
	int jumpsLeft = 1;
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	Collision(Entity& other) { this->other = other; };
};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

// Sets the brightness of the screen
struct ScreenState
{
	float darken_screen_factor = -1;
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// A timer that will be associated to dying chicken
struct DeathTimer
{
	float counter_ms = 3000;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & chicken.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = {1,1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

struct BezierProjectile {
	vec2 targetPosition;
	vec2 startPosition;
	float elapsedTime = 0.0f;
	vec2 controlPoint;
};

struct Archer {

};

struct ArrowCooldown {
	float timeSinceLastShot = 0.0f;
	float cooldown = 3000;
};

struct Particle {
	vec2 position;
	vec2 velocity;
	vec4 color;
	float life;
};

struct ParticleEmitter {
	vec2 emission_point;
	int particles_per_second;
	vec2 initial_velocity;
	vec4 color;
	float lifespan;
};

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
	EARTH = 0,
	PLATFORM = EARTH + 1,
	BOULDER = PLATFORM + 1,
	CHASEBOULDER = BOULDER + 1,
	CHECKPOINT = CHASEBOULDER + 1,
	LEVELEND = CHECKPOINT + 1,
	OLIVER = LEVELEND + 1,
	RUN1 = OLIVER + 1,
	RUN2 = RUN1 + 1,
	RUN3 = RUN2 + 1,
	RUN4 = RUN3 + 1,
	RUN5 = RUN4 + 1,
	RUN6 = RUN5 + 1,
	PENCIL = RUN6 + 1,
	PAINTCAN = PENCIL + 1,
	TUTORIAL = PAINTCAN + 1,
	BACKGROUND = TUTORIAL + 1,
	SPIKES = BACKGROUND + 1,
	EMPTY = SPIKES + 1,
	HINT1 = EMPTY + 1,
	HINT2 = HINT1 + 1,
	HINT3 = HINT2 + 1,
	HINT4 = HINT3 + 1,
	HINT5 = HINT4 + 1,
	HINT6 = HINT5 + 1,
	HINT7 = HINT6 + 1,
	HINT8 = HINT7 + 1,
	GREENENEMY = HINT8 + 1,
	BEZIERPROJECTILE = GREENENEMY + 1,
	CIRCLEPARTICLE = BEZIERPROJECTILE + 1,
	TEXTURE_COUNT = CIRCLEPARTICLE + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	EGG = COLOURED + 1,
	CHICKEN = EGG + 1,
	TEXTURED = CHICKEN + 1,
	WIND = TEXTURED + 1,
	BACKGROUND = WIND + 1,
	EFFECT_COUNT = BACKGROUND + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	CHICKEN = 0,
	OLIVER = CHICKEN + 1,
	SPRITE = OLIVER + 1,
	EGG = SPRITE + 1,
	DRAWN_LINE = EGG + 1,
	DEBUG_LINE = DRAWN_LINE + 1,
	JOINT_TRIANGLE = DEBUG_LINE + 1,
	SCREEN_TRIANGLE = JOINT_TRIANGLE + 1,
	GEOMETRY_COUNT = SCREEN_TRIANGLE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
};

