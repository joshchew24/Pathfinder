#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"

// matrices
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//fonts
#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>

struct Character {
	unsigned int TextureID;  // ID handle of the glyph texture
	glm::ivec2   Size;       // Size of glyph
	glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
	unsigned int Advance;    // Offset to advance to next glyph
	char character;
};

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	const std::vector < std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
	{
		  std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::CHICKEN, mesh_path("chicken.obj")),
		  std::pair<GEOMETRY_BUFFER_ID, std::string> (GEOMETRY_BUFFER_ID::OLIVER, mesh_path("oliver.obj"))
		  // specify meshes of other assets here
	};

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, texture_count> texture_paths = {
			textures_path("earthBlock.png"),
			textures_path("platform.png"),
			textures_path("Free/Traps/Rock Head/Idle.png"),
			textures_path("chaseBoulder.png"),
			textures_path("idleFlag.png"),
			textures_path("end.png"),
			textures_path("idle.png"),
			textures_path("Run1.png"),
			textures_path("Run2.png"),
			textures_path("Run3.png"),
			textures_path("Run4.png"),
			textures_path("pencil.png"),
			textures_path("paintcan.png"),
			textures_path("tutorial.png"),
			textures_path("background0.png"),
			textures_path("Free/Traps/Spikes/idle.png"),
			textures_path("Empty.png"),
			textures_path("hintAnimation/hint1.png"),
			textures_path("hintAnimation/hint2.png"),
			textures_path("hintAnimation/hint3.png"),
			textures_path("hintAnimation/hint4.png"),
			textures_path("hintAnimation/hint5.png"),
			textures_path("hintAnimation/hint6.png"),
			textures_path("hintAnimation/hint7.png"),
			textures_path("hintAnimation/hint8.png"),
			textures_path("character_roundGreen.png"),
			textures_path("tile_0008.png")
      };

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"),
		shader_path("egg"),
		shader_path("chicken"),
		shader_path("textured"),
		shader_path("wind"),
		shader_path("background")};

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

public:
	// Initialize the window
	bool init(GLFWwindow* window);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();

	void initializeGlEffects();

	void initializeGlMeshes();
	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();
	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the wind
	// shader
	bool initScreenTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw();

	mat3 createProjectionMatrix();

	void RenderSystem::initParallaxRendering();

	void RenderSystem::initIntroduction();

	void RenderSystem::drawBackground();

	bool RenderSystem::loadTextures(const char* fileName, GLuint &textureId);

	float camera_x = 0.0f;
	float camera_y = 0.0f;

	bool RenderSystem::fontInit(GLFWwindow& window, const std::string& font_filename, unsigned int font_default_size);

	void RenderSystem::renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color, const glm::mat4& trans);

	static bool introductionScreen;

	void RenderSystem::renderIntroduction(int i);

	void RenderSystem::renderHelper(float transX, float transY, float textX, float textY, float oliverTransparency, float oldTransparency, GLuint dialogue, const char* text, float textScale);

	int sceneIndex = 0;
private:
	// Internal drawing functions for each entity type
	void drawTexturedMesh(Entity entity, const mat3& projection);
	void drawToScreen();

	// Window handle
	GLFWwindow* window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	Entity screen_state_entity;

	GLuint far_clouds;
	GLuint near_clouds;
	GLuint sky;
	GLuint far_mountains;
	GLuint mountains;
	GLuint trees;
	GLuint backGroundVao;
	GLuint backGroundVbo;
	GLuint backGroundShader;

	GLuint globalVao;

	//font
	std::map<char, Character> m_ftCharacters;
	GLuint m_font_shaderProgram;
	GLuint m_font_VAO;
	GLuint m_font_VBO;
	glm::mat4 trans = glm::mat4(1.0f);

	//introduction
	GLuint dialogueBoxLeft;
	GLuint dialogueBoxRight;
	GLuint oliverPixel;
	GLuint oldManPixel;
	GLuint introductionVao;
	GLuint introductionVbo;
	GLuint introductionShader;
};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
