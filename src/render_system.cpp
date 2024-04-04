// internal
#include "render_system.hpp"
#include <SDL.h>
#include <SDL_opengl.h>
#include "tiny_ecs_registry.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <chrono>
#include <thread>

bool RenderSystem::introductionScreen = false;

bool RenderSystem::endScreen = false;

void RenderSystem::drawTexturedMesh(Entity entity,
									const mat3 &projection)
{
	Motion &motion = registry.motions.get(entity);
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(motion.position);
	transform.rotate(motion.angle);
	transform.scale(motion.scale);

	assert(registry.renderRequests.has(entity));
	const RenderRequest &render_request = registry.renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::CHICKEN || render_request.used_effect == EFFECT_ASSET_ID::EGG)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)sizeof(vec3));
		gl_has_errors();

		if (render_request.used_effect == EFFECT_ASSET_ID::CHICKEN)
		{
			// Light up?
			GLint light_up_uloc = glGetUniformLocation(program, "light_up");
			assert(light_up_uloc >= 0);

			// !!! TODO A1: set the light_up shader variable using glUniform1i,
			// similar to the glUniform1f call below. The 1f or 1i specified the type, here a single int.
			gl_has_errors();
		}
	}
	else
	{
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	glUniform3fv(color_uloc, 1, (float *)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();
	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

void RenderSystem::drawBackground() {
	glUseProgram(backGroundShader);
	glUniform2f(glGetUniformLocation(backGroundShader, "camera"), camera_x, camera_y);

	glBindVertexArray(backGroundVao);

	glUniform1f(glGetUniformLocation(backGroundShader, "parallaxFactor"), 0.001);
	glUniform1i(glGetUniformLocation(backGroundShader, "texture1"), 2);
	glUniform1f(glGetUniformLocation(backGroundShader, "transparency"), 1);
	glm::vec3 translation1(0.0f, 0.14f, 0.0f);
	glUniform3fv(glGetUniformLocation(backGroundShader, "transform"), 1, glm::value_ptr(translation1));
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, sky);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	gl_has_errors();

	glUniform1f(glGetUniformLocation(backGroundShader, "parallaxFactor"), 0.002);
	glUniform1i(glGetUniformLocation(backGroundShader, "texture1"), 0);
	glUniform1f(glGetUniformLocation(backGroundShader, "transparency"), 3);
	glm::vec3 translation(0.0f, 0.245f, 0.0f);
	glUniform3fv(glGetUniformLocation(backGroundShader, "transform"), 1, glm::value_ptr(translation));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, far_clouds);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	gl_has_errors();

	glUniform1f(glGetUniformLocation(backGroundShader, "parallaxFactor"), 0.001);
	glUniform1i(glGetUniformLocation(backGroundShader, "texture1"), 1);
	glUniform1f(glGetUniformLocation(backGroundShader, "transparency"), 1);
	glm::vec3 translation0(0.0f, 0.0f, 0.0f);
	glUniform3fv(glGetUniformLocation(backGroundShader, "transform"), 1, glm::value_ptr(translation0));
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, far_mountains);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	gl_has_errors();

	glUniform1f(glGetUniformLocation(backGroundShader, "parallaxFactor"), 0.006);
	glUniform1i(glGetUniformLocation(backGroundShader, "texture1"), 3);
	glUniform1f(glGetUniformLocation(backGroundShader, "transparency"), 1.5);
	glm::vec3 translation2(0.0f, 0.23f, 0.0f);
	glUniform3fv(glGetUniformLocation(backGroundShader, "transform"), 1, glm::value_ptr(translation2));
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, near_clouds);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	gl_has_errors();

	glUniform1f(glGetUniformLocation(backGroundShader, "parallaxFactor"), 0.0017);
	glUniform1i(glGetUniformLocation(backGroundShader, "texture1"), 4);
	glUniform1f(glGetUniformLocation(backGroundShader, "transparency"), 1.6);
	glm::vec3 translation3(0.0f, -0.2f, 0.0f);
	glUniform3fv(glGetUniformLocation(backGroundShader, "transform"), 1, glm::value_ptr(translation3));
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, mountains);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	gl_has_errors();

	glUniform1f(glGetUniformLocation(backGroundShader, "parallaxFactor"), 0.023);
	glUniform1i(glGetUniformLocation(backGroundShader, "texture1"), 5);
	glUniform1f(glGetUniformLocation(backGroundShader, "transparency"), 1);
	glm::vec3 translation4(0.0f, -0.12f, 0.0f);
	glUniform3fv(glGetUniformLocation(backGroundShader, "transform"), 1, glm::value_ptr(translation4));
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, trees);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	gl_has_errors();

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	gl_has_errors();

	glUseProgram(0);
}

// draw the intermediate texture to the screen, with some distortion to simulate
// wind
void RenderSystem::drawToScreen()
{
	// Setting shaders
	// get the wind texture, sprite mesh, and program
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::WIND]);
	gl_has_errors();
	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(1.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	// Enabling alpha channel for textures
	glDisable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
																	 // indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();
	const GLuint wind_program = effects[(GLuint)EFFECT_ASSET_ID::WIND];
	// Set clock
	GLuint time_uloc = glGetUniformLocation(wind_program, "time");
	GLuint dead_timer_uloc = glGetUniformLocation(wind_program, "darken_screen_factor");
	glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));
	ScreenState &screen = registry.screenStates.get(screen_state_entity);
	glUniform1f(dead_timer_uloc, screen.darken_screen_factor);
	gl_has_errors();
	// Set the vertex position and vertex texture coordinates (both stored in the
	// same VBO)
	GLint in_position_loc = glGetAttribLocation(wind_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);

	gl_has_errors();
	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr); // one triangle = 3 vertices; nullptr indicates that there is
				  // no offset from the bound index buffer
	gl_has_errors();
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw()
{
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();
	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	glClearColor(0.674, 0.847, 1.0 , 1.0);
	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
							  // and alpha blending, one would have to sort
							  // sprites back to front
	gl_has_errors();
	mat3 projection_2D = createProjectionMatrix();

	if (introductionScreen) {
		int w, h;
		glfwGetFramebufferSize(window, &w, &h);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, w, h);
		glDepthRange(0, 10);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearDepth(1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		gl_has_errors();
		glDisable(GL_DEPTH_TEST);

		renderIntroduction(sceneIndex);
	}
	else if (endScreen) {
		int w, h;
		glfwGetFramebufferSize(window, &w, &h);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, w, h);
		glDepthRange(0, 10);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearDepth(1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		gl_has_errors();
		glDisable(GL_DEPTH_TEST);

		renderEnding(sceneIndex);
	}
	else {
		drawBackground();

		glBindVertexArray(globalVao);
		for (Entity entity : registry.renderRequests.entities)
		{
			if (!registry.motions.has(entity))
				continue;
			// Note, its not very efficient to access elements indirectly via the entity
			// albeit iterating through all Sprites in sequence. A good point to optimize
			drawTexturedMesh(entity, projection_2D);
		}

		// Truely render to the screen
		drawToScreen();

		// renderHint
		renderText(hint, hintPos.x - 100, hintPos.y - 70, 0.6, glm::vec3(1.0f, 1.0f, 1.0f), trans);

		if (renderMainMenuText) {
			renderText("PLAY", window_width_px / 2 - 75 , window_height_px / 2 + 90, 1.7, glm::vec3(0,0,0), trans);
			renderText("RESTART", window_width_px / 2 - 110, window_height_px / 2 - 50, 1.54, glm::vec3(0, 0, 0), trans);
			renderText("EXIT", window_width_px / 2 - 75, window_height_px / 2 - 200, 1.7, glm::vec3(0, 0, 0), trans);
		}
	}

	gl_has_errors();
}


void RenderSystem::renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color, const glm::mat4& trans) {

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// activate the shaders!
	glUseProgram(m_font_shaderProgram);

	unsigned int textColor_location =
		glGetUniformLocation(
			m_font_shaderProgram,
			"textColor"
		);
	assert(textColor_location >= 0);
	glUniform3f(textColor_location, color.x, color.y, color.z);

	auto transform_location = glGetUniformLocation(
		m_font_shaderProgram,
		"transform"
	);
	assert(transform_location > -1);
	glUniformMatrix4fv(transform_location, 1, GL_FALSE, glm::value_ptr(trans));

	glBindVertexArray(m_font_VAO);

	// iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = m_ftCharacters[*c];

		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;

		// update VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};

		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// std::cout << "binding texture: " << ch.character << " = " << ch.TextureID << std::endl;

		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, m_font_VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

}

mat3 RenderSystem::createProjectionMatrix()
{
	// Fake projection matrix, scales with respect to window coordinates
	float left = 0.f;
	float top = 0.f;

	gl_has_errors();
	float right = (float) window_width_px;
	float bottom = (float) window_height_px;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
}

void RenderSystem::renderHelper(float transX, float transY, float textX, float textY,  float oliverTransparency, float oldTransparency, GLuint dialogue, const char* text, float textScale) {
	glUseProgram(introductionShader);
	glBindVertexArray(introductionVao);

	glUniform1i(glGetUniformLocation(introductionShader, "texture1"), 1);
	glm::vec3 translation1(-1.8f, 0.1f, 0.0f);
	glUniform3fv(glGetUniformLocation(introductionShader, "transform"), 1, glm::value_ptr(translation1));
	glm::vec3 scale1(0.3f, 1.0f, 1.0f);
	glUniform3fv(glGetUniformLocation(introductionShader, "scale"), 1, glm::value_ptr(scale1));
	glUniform1f(glGetUniformLocation(introductionShader, "transparency"), oliverTransparency);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, oliverPixel);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glUniform1i(glGetUniformLocation(introductionShader, "texture1"), 2);
	glm::vec3 translation2(0.7f, 0.1f, 0.0f);
	glUniform3fv(glGetUniformLocation(introductionShader, "transform"), 1, glm::value_ptr(translation2));
	glm::vec3 scale2(1.0f, 1.2f, 1.0f);
	glUniform3fv(glGetUniformLocation(introductionShader, "scale"), 1, glm::value_ptr(scale2));
	glUniform1f(glGetUniformLocation(introductionShader, "transparency"), oldTransparency);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, oldManPixel);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	gl_has_errors();

	glUniform1i(glGetUniformLocation(introductionShader, "texture1"), 0);
	glm::vec3 translation0(transX, transY, 0.0f);
	glUniform3fv(glGetUniformLocation(introductionShader, "transform"), 1, glm::value_ptr(translation0));
	glm::vec3 scale0(1.3f, 0.7f, 1.0f);
	glUniform3fv(glGetUniformLocation(introductionShader, "scale"), 1, glm::value_ptr(scale0));
	glUniform1f(glGetUniformLocation(introductionShader, "transparency"), 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, dialogue);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	renderText(text, textX, textY, textScale, glm::vec3(1.0f, 1.0f, 1.0f), trans);
	renderText("Press Z to skip", textX + 75, textY - 75, 0.5, glm::vec3(0.0f, 0.0f, 0.0f), trans);
	renderText("Left click to progress", textX + 75, textY - 50, 0.5, glm::vec3(0.0f, 0.0f, 0.0f), trans);
}

void RenderSystem::renderIntroduction(int i) {
	if (i == 0) {
		renderHelper(-0.23f, -0.7f, 140, 200, 1, 3, dialogueBoxLeft, "wha.. what happened?", 2);
	}
	else if (i == 1) {
		renderHelper(-0.23f, -0.7f, 140, 200, 1, 3, dialogueBoxLeft, "where am I?", 2);
	}
	else if (i == 2) {
		renderHelper(0.25f, -0.7f, 800, 200, 3, 1, dialogueBoxRight, "Hello, Oliver.", 2);
	}
	else if (i == 3) {
		renderHelper(0.25f, -0.7f, 800, 200, 3, 1, dialogueBoxRight, "I summoned you into this world.", 1.7);
	}
	else if (i == 4) {
		renderHelper(0.25f, -0.7f, 800, 200, 3, 1, dialogueBoxRight, "I want you to test a few things for me.", 1.5);
	}
	else if (i == 5) {
		renderHelper(0.25f, -0.7f, 800, 200, 3, 1, dialogueBoxRight, "You will enter a magical pixel world.", 1.5);
	}
	else if (i == 6) {
		renderHelper(0.25f, -0.7f, 800, 200, 3, 1, dialogueBoxRight, "All you have to do is reach the trophy at each level.", 1.11);
	}
	else if (i == 7) {
		renderHelper(-0.23f, -0.7f, 140, 200, 1, 3, dialogueBoxLeft, "This is crazy!!", 2);
	}
	else if (i == 8) {
		renderHelper(0.25f, -0.7f, 800, 200, 3, 1, dialogueBoxRight, "Oh, there will also be enemies.", 1.5);
	}
	else if (i == 9) {
		renderHelper(-0.23f, -0.7f, 140, 200, 1, 3, dialogueBoxLeft, "WHAT", 2);
	}
	else if (i == 10) {
		renderHelper(0.25f, -0.7f, 800, 200, 3, 1, dialogueBoxRight, "You will also turn into a stickman", 1.5);
	}
	else if (i == 11) {
		renderHelper(0.25f, -0.7f, 800, 200, 3, 1, dialogueBoxRight, "Goodluck.", 1.7);
	}
	else if (i == 12) {
		renderHelper(-0.23f, -0.7f, 140, 200, 1, 3, dialogueBoxLeft, "wait, I still have ques....", 1.5);
	}

}

void RenderSystem::renderEnding(int i) {
	if (i == 0) {
		renderHelper(-0.23f, -0.7f, 140, 200, 1, 3, dialogueBoxLeft, "I.. I did it??", 2);
	}
	else if (i == 1) {
		renderHelper(-0.23f, -0.7f, 140, 200, 1, 3, dialogueBoxLeft, "I DID IT!!!!", 2);
	}
	else if (i == 2) {
		renderHelper(0.25f, -0.7f, 800, 200, 3, 1, dialogueBoxRight, "Good job, Oliver.", 2);
	}
	else if (i == 3) {
		renderHelper(0.25f, -0.7f, 800, 200, 3, 1, dialogueBoxRight, "You done a very good job.", 1.7);
	}
	else if (i == 4) {
		renderHelper(0.25f, -0.7f, 800, 200, 3, 1, dialogueBoxRight, "I have learned alot from this experiment.", 1.2);
	}
	else if (i == 5) {
		renderHelper(0.25f, -0.7f, 800, 200, 3, 1, dialogueBoxRight, "Now, I want you to wait until I finish the next stage", 1.1);
	}
	else if (i == 6) {
		renderHelper(-0.23f, -0.7f, 140, 200, 1, 3, dialogueBoxLeft, "wait what, there is more??", 1.7);
	}
	else if (i == 7) {
		renderHelper(0.25f, -0.7f, 800, 200, 3, 1, dialogueBoxRight, "Of course, we just merely got started...", 1.3);
	}
	else if (i == 8) {
		renderHelper(0.25f, -0.7f, 800, 200, 3, 1, dialogueBoxRight, "There will be new enemies, new levels.", 1.5);
	}
	else if (i == 9) {
		renderHelper(-0.23f, -0.7f, 140, 200, 1, 3, dialogueBoxLeft, "Do I at least become a human again?", 1.5);
	}
	else if (i == 10) {
		renderHelper(0.25f, -0.7f, 800, 200, 3, 1, dialogueBoxRight, "No, You will still be a stickman", 1.5);
	}
	else if (i == 11) {
		renderHelper(0.25f, -0.7f, 800, 200, 3, 1, dialogueBoxRight, "Now repeat these stages until I make new ones", 1.2);
	}
	else if (i == 12) {
		renderHelper(-0.23f, -0.7f, 140, 200, 1, 3, dialogueBoxLeft, "..............", 1.5);
	}

}