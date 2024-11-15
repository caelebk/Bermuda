// internal
#include "render_system.hpp"

#include <SDL.h>

#include "misc.hpp"
#include "oxygen_system.hpp"
#include "tiny_ecs_registry.hpp"

void RenderSystem::drawTexturedMesh(Entity entity, const mat3& projection) {
  Position& position = registry.positions.get(entity);
  // Transformation code, see Rendering and Transformation in the template
  // specification for more info Incrementally updates transformation matrix,
  // thus ORDER IS IMPORTANT
  Transform transform;
  transform.translate(position.position);
  transform.rotate(position.angle);
  transform.scale(position.scale);

  assert(registry.renderRequests.has(entity));
  const RenderRequest& render_request = registry.renderRequests.get(entity);

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
  if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED ||
      render_request.used_effect == EFFECT_ASSET_ID::TEXTURED_OXYGEN ||
      render_request.used_effect == EFFECT_ASSET_ID::PLAYER ||
      render_request.used_effect == EFFECT_ASSET_ID::ENEMY) {
    GLint in_position_loc = glGetAttribLocation(program, "in_position");
    GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
    gl_has_errors();
    assert(in_texcoord_loc >= 0);

    glEnableVertexAttribArray(in_position_loc);
    glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
                          sizeof(TexturedVertex), (void*)0);
    gl_has_errors();

    glEnableVertexAttribArray(in_texcoord_loc);
    glVertexAttribPointer(
        in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
        (void*)sizeof(
            vec3));  // note the stride to skip the preceeding vertex position

    if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED_OXYGEN) {
      GLuint time_uloc = glGetUniformLocation(program, "time");
      GLuint is_low_oxygen_uloc =
          glGetUniformLocation(program, "is_low_oxygen");
      gl_has_errors();
      glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));
      glUniform1i(is_low_oxygen_uloc, registry.lowOxygen.has(entity));

      gl_has_errors();
    }

    if (render_request.used_effect == EFFECT_ASSET_ID::PLAYER ||
        render_request.used_effect == EFFECT_ASSET_ID::ENEMY) {
      GLuint damage_timer_uloc = glGetUniformLocation(program, "damageTimer");
      GLuint stun_timer_uloc   = glGetUniformLocation(program, "stunned");
      GLuint angry_timer_uloc  = glGetUniformLocation(program, "is_angry");
      gl_has_errors();
      glUniform1f(damage_timer_uloc, registry.attacked.has(entity)
                                         ? registry.attacked.get(entity).timer
                                         : 0.0f);
      glUniform1f(stun_timer_uloc, registry.stunned.has(entity) ? true : false);
      if (entity == player_weapon) {
        glUniform1f(stun_timer_uloc,
                    registry.stunned.has(player) ? true : false);
        glUniform1f(damage_timer_uloc, registry.attacked.has(player)
                                           ? registry.attacked.get(player).timer
                                           : 0.0f);
      }
      glUniform1f(angry_timer_uloc, registry.bosses.has(entity)
                                        ? registry.bosses.get(entity).is_angry
                                        : false);
      gl_has_errors();
    }

    // Enabling and binding texture to slot 0
    glActiveTexture(GL_TEXTURE0);
    gl_has_errors();

    assert(registry.renderRequests.has(entity));
    GLuint texture_id =
        texture_gl_handles[(GLuint)registry.renderRequests.get(entity)
                               .used_texture];

    glBindTexture(GL_TEXTURE_2D, texture_id);
    gl_has_errors();
  } else if (render_request.used_effect == EFFECT_ASSET_ID::COLLISION_MESH) {
    GLint in_position_loc = glGetAttribLocation(program, "in_position");
    GLint in_color_loc    = glGetAttribLocation(program, "in_color");
    gl_has_errors();

    glEnableVertexAttribArray(in_position_loc);
    glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
                          sizeof(ColoredVertex), (void*)0);
    gl_has_errors();
    glEnableVertexAttribArray(in_color_loc);
    glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
                          sizeof(ColoredVertex), (void*)sizeof(vec3));
    gl_has_errors();
  } else {
    assert(false && "Type of render request not supported");
  }

  // Getting uniform locations for glUniform* calls
  GLint      color_uloc = glGetUniformLocation(program, "fcolor");
  const vec3 color =
      registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
  glUniform3fv(color_uloc, 1, (float*)&color);
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
  glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*)&transform.mat);
  GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
  glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projection);
  gl_has_errors();
  // Drawing of num_indices/3 triangles specified in the index buffer
  glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
  gl_has_errors();
}

// draw the intermediate texture to the screen, with some distortion to simulate
// water
void RenderSystem::drawToScreen() {
  // Setting shaders
  // get the water texture, sprite mesh, and program
  glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::WATER]);
  gl_has_errors();
  // Clearing backbuffer
  int w, h;
  glfwGetFramebufferSize(window, &w,
                         &h);  // Note, this will be 2x the resolution given to
                               // glfwCreateWindow on retina displays
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
  glBindBuffer(GL_ARRAY_BUFFER,
               vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
  glBindBuffer(
      GL_ELEMENT_ARRAY_BUFFER,
      index_buffers[(GLuint)GEOMETRY_BUFFER_ID::
                        SCREEN_TRIANGLE]);  // Note, GL_ELEMENT_ARRAY_BUFFER
                                            // associates indices to the bound
                                            // GL_ARRAY_BUFFER
  gl_has_errors();
  const GLuint water_program = effects[(GLuint)EFFECT_ASSET_ID::WATER];
  // Set clock
  GLuint time_uloc = glGetUniformLocation(water_program, "time");
  GLuint dead_timer_uloc =
      glGetUniformLocation(water_program, "darken_screen_factor");
  glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));
  ScreenState& screen = registry.screenStates.get(screen_state_entity);
  glUniform1f(dead_timer_uloc, screen.darken_screen_factor);
  GLuint is_paused_uloc = glGetUniformLocation(water_program, "is_paused");
  glUniform1i(is_paused_uloc, paused);
  gl_has_errors();
  // Set the vertex position and vertex texture coordinates (both stored in the
  // same VBO)
  GLint in_position_loc = glGetAttribLocation(water_program, "in_position");
  glEnableVertexAttribArray(in_position_loc);
  glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3),
                        (void*)0);
  gl_has_errors();

  // Bind our texture in Texture Unit 0
  glActiveTexture(GL_TEXTURE0);

  glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
  gl_has_errors();
  // Draw
  glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
                 nullptr);  // one triangle = 3 vertices; nullptr indicates that
                            // there is no offset from the bound index buffer
  gl_has_errors();
}

void RenderSystem::renderText(std::string text, float x, float y, float scale,
                              const glm::vec3& color, const glm::mat4& trans) {
  // activate the shader program
  glEnable(GL_BLEND);
  glUseProgram(font_shaderProgram);

  // get shader uniforms
  GLint textColor_location =
      glGetUniformLocation(font_shaderProgram, "textColor");
  glUniform3f(textColor_location, color.x, color.y, color.z);

  GLint transformLoc = glGetUniformLocation(font_shaderProgram, "transform");
  glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

  glBindVertexArray(font_VAO);

  // iterate through all characters
  std::string::const_iterator c;
  for (c = text.begin(); c != text.end(); c++) {
    Character ch = fontCharacters[*c];

    float xpos = x + ch.Bearing.x * scale;
    float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

    float w = ch.Size.x * scale;
    float h = ch.Size.y * scale;
    // update VBO for each character
    float vertices[6][4] = {
        {xpos, ypos + h, 0.0f, 0.0f},    {xpos, ypos, 0.0f, 1.0f},
        {xpos + w, ypos, 1.0f, 1.0f},

        {xpos, ypos + h, 0.0f, 0.0f},    {xpos + w, ypos, 1.0f, 1.0f},
        {xpos + w, ypos + h, 1.0f, 0.0f}};

    // render glyph texture over quad
    glBindTexture(GL_TEXTURE_2D, ch.TextureID);
    // std::cout << "binding texture: " << ch.character << " = " << ch.TextureID
    // << std::endl;

    // update content of VBO memory
    glBindBuffer(GL_ARRAY_BUFFER, font_VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // render quad
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // now advance cursors for next glyph (note that advance is number of 1/64
    // pixels)
    x += (ch.Advance >> 6) *
         scale;  // bitshift by 6 to get value in pixels (2^6 = 64)
  }

  glBindVertexArray(vao);
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_BLEND);
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw() {
  // Getting size of window
  int w, h;
  glfwGetFramebufferSize(window, &w,
                         &h);  // Note, this will be 2x the resolution given to
                               // glfwCreateWindow on retina displays

  // First render to the custom framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
  gl_has_errors();
  // Clearing backbuffer
  glViewport(0, 0, w, h);
  glDepthRange(0.00001, 10);
  glClearColor(GLfloat(172 / 255), GLfloat(216 / 255), GLfloat(255 / 255), 1.0);
  glClearDepth(10.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_DEPTH_TEST);  // native OpenGL does not work with a depth buffer
                             // and alpha blending, one would have to sort
                             // sprites back to front
  gl_has_errors();
  mat3 projection_2D = createProjectionMatrix();

  //////////////////////////////////////////////////////////////////////////////////////
  /*************************************************************************************
   * Draw All Textured Meshes that have a Position and Size Component
   *
   * @details Order of for loops determines "depth" of render during overlap for
   *now Note, its not very efficient to access elements indirectly via the
   *entity albeit iterating through all Sprites in sequence. A good point to
   *optimize
   *************************************************************************************/
  for (Entity floor : registry.floors.entities) {
    if (registry.renderRequests.has(floor))
      drawTexturedMesh(floor, projection_2D);
  }
  for (Entity wall : registry.activeWalls.entities) {
    if (registry.renderRequests.has(wall)) {
      if (registry.breakables.has(wall) && registry.oxygen.has(wall)) {
        Oxygen& wallOxygen = registry.oxygen.get(wall);
        if (registry.renderRequests.has(wallOxygen.backgroundBar) &&
            registry.renderRequests.has(wallOxygen.oxygenBar)) {
          drawTexturedMesh(wallOxygen.backgroundBar, projection_2D);
          drawTexturedMesh(wallOxygen.oxygenBar, projection_2D);
        }
      }
      drawTexturedMesh(wall, projection_2D);
    }
  }
  for (Entity door : registry.activeDoors.entities) {
    if (registry.renderRequests.has(door))
      drawTexturedMesh(door, projection_2D);
  }
  for (Entity consumable : registry.consumables.entities) {
    if (registry.renderRequests.has(consumable))
      drawTexturedMesh(consumable, projection_2D);
  }
  for (Entity interactable : registry.interactable.entities) {
    if (registry.renderRequests.has(interactable))
      drawTexturedMesh(interactable, projection_2D);
  }
  for (Entity player : registry.players.entities) {
    if (registry.renderRequests.has(player))
      drawTexturedMesh(player, projection_2D);
  }
  // Collision mesh rendering
  for (Entity playerCollisionMesh : registry.playersCollisionMeshes.entities) {
    if (registry.renderRequests.has(playerCollisionMesh)) {
      drawTexturedMesh(playerCollisionMesh, projection_2D);
    }
  }
  for (Entity projectile : registry.playerProjectiles.entities) {
    if (registry.renderRequests.has(projectile))
      drawTexturedMesh(projectile, projection_2D);
  }
  for (Entity weapon : registry.playerWeapons.entities) {
    if (registry.renderRequests.has(weapon))
      drawTexturedMesh(weapon, projection_2D);
  }
  for (Entity enemy : registry.deadlys.entities) {
    if (registry.renderRequests.has(enemy))
      drawTexturedMesh(enemy, projection_2D);
  }
  for (Entity enemy : registry.deadlys.entities) {
    if (registry.oxygen.has(enemy)) {
      Oxygen& enemyOxygen = registry.oxygen.get(enemy);
      if (registry.renderRequests.has(enemyOxygen.backgroundBar) &&
          registry.renderRequests.has(enemyOxygen.oxygenBar)) {
        drawTexturedMesh(enemyOxygen.backgroundBar, projection_2D);
        drawTexturedMesh(enemyOxygen.oxygenBar, projection_2D);
      }
      if (registry.emoting.has(enemy)) {
        Emoting& emote = registry.emoting.get(enemy);
        if (registry.renderRequests.has(emote.child)) {
          drawTexturedMesh(emote.child, projection_2D);
        }
      }
    }
  }
  for (Entity playerHUDElement : registry.playerHUD.entities) {
    if (registry.renderRequests.has(playerHUDElement))
      drawTexturedMesh(playerHUDElement, projection_2D);
  }
  for (Entity cursor : registry.cursors.entities) {
    if (registry.renderRequests.has(cursor))
      drawTexturedMesh(cursor, projection_2D);
  }
  for (Entity pauseMenu : registry.pauseMenus.entities) {
    if (registry.renderRequests.has(pauseMenu))
      drawTexturedMesh(pauseMenu, projection_2D);
  }
  //////////////////////////////////////////////////////////////////////////////////////

  // Render Fonts
  if (!paused) {
    for (Entity entity : registry.textRequests.entities) {
      if (!registry.positions.has(entity) || !registry.colors.has(entity))
        continue;
      TextRequest& textRequest = registry.textRequests.get(entity);
      Position&    position    = registry.positions.get(entity);
      vec3&        color       = registry.colors.get(entity);

      Transform transform;
      transform.translate(position.position);
      transform.rotate(position.angle);
      transform.scale(position.scale);

      renderText(textRequest.text, position.position.x,
                 abs(position.position.y - window_height_px),
                 textRequest.textScale, color, transform.mat);
    }
  }

  // Truely render to the screen
  drawToScreen();

  // flicker-free display with a double buffer
  glfwSwapBuffers(window);
  gl_has_errors();
}

mat3 RenderSystem::createProjectionMatrix() {
  // Fake projection matrix, scales with respect to window coordinates
  float left = 0.f;
  float top  = 0.f;

  gl_has_errors();
  float right  = (float)window_width_px;
  float bottom = (float)window_height_px;

  float sx = 2.f / (right - left);
  float sy = 2.f / (top - bottom);
  float tx = -(right + left) / (right - left);
  float ty = -(top + bottom) / (top - bottom);
  return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
}
