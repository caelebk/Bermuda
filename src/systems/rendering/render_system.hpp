#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"

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
  std::array<ivec2, texture_count>  texture_dimensions;

  // Make sure these paths remain in sync with the associated enumerators.
  // Associated id with .obj path
  const std::vector<std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths = {
      // specify meshes of other assets here
		  std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::PLAYER, mesh_path("player.obj"))
  };

  // Make sure these paths remain in sync with the associated enumerators.
  const std::array<std::string, texture_count> texture_paths = {
      textures_path("pause/pause_menu.png"),
      textures_path("numbers/0.png"),
      textures_path("numbers/1.png"),
      textures_path("numbers/2.png"),
      textures_path("numbers/3.png"),
      textures_path("numbers/4.png"),
      textures_path("numbers/5.png"),
      textures_path("numbers/6.png"),
      textures_path("numbers/7.png"),
      textures_path("numbers/8.png"),
      textures_path("numbers/9.png"),
      textures_path("cursors/crosshair_cursor.png"),
      textures_path("player/player1.png"),
      textures_path("player/player2.png"),
      textures_path("player/player3.png"),
      textures_path("player/player_dash.png"),
      textures_path("player/harpoon_gun.png"),
      textures_path("player/harpoon.png"),
      textures_path("player/net_gun.png"),
      textures_path("player/net.png"),
      textures_path("player/concussive_gun.png"),
      textures_path("player/concussive.png"),
      textures_path("player/torpedo_gun.png"),
      textures_path("player/torpedo.png"),
      textures_path("player/shrimp_gun.png"), 
      textures_path("player/shrimp.png"), // TODO: shrimp projectile needed, this is for HUD
      textures_path("consumables/key.png"),
      textures_path("oxygen_and_health/player_oxygen_tank.png"),
      textures_path("oxygen_and_health/player_oxygen_bar.png"),
      textures_path("oxygen_and_health/enemy_background_bar.png"),
      textures_path("oxygen_and_health/enemy_oxygen_bar.png"),
      textures_path("enemy/exclamation.png"),
      textures_path("enemy/question.png"),
      textures_path("map_non_interactable/wall.png"),
      textures_path("map_non_interactable/floor.png"),
      textures_path("map_interactable/locked_door_h.png"),
      textures_path("map_interactable/locked_door_v.png"),
      textures_path("map_interactable/locked_yellow_h.png"),
      textures_path("map_interactable/locked_yellow_v.png"),
      textures_path("map_interactable/locked_blue_h.png"),
      textures_path("map_interactable/locked_blue_v.png"),
      textures_path("map_interactable/locked_red_h.png"),
      textures_path("map_interactable/locked_red_v.png"),
      textures_path("map_interactable/doorway_h.png"),
      textures_path("map_interactable/doorway_v.png"),
      textures_path("enemy/jellyfish.png"),
      textures_path("enemy/fish0.png"),
      textures_path("enemy/fish1.png"),
      textures_path("enemy/fish2.png"),
      textures_path("enemy/fish3.png"),
      textures_path("enemy/fish4.png"),
      textures_path("enemy/shark.png"),
      textures_path("enemy/krab.png"),       // source: Vecteezy.com
      textures_path("enemy/krab_boss.png"),  // source: Vecteezy.com
      textures_path("enemy/krab_boss_chase.png"),
      textures_path("map_interactable/breakable_crate.png"),
      textures_path("consumables/oxygen_canister.png"),
      textures_path("map_interactable/geyser.png"),
  };

  std::array<GLuint, effect_count> effects;
  // Make sure these paths remain in sync with the associated enumerators.
  const std::array<std::string, effect_count> effect_paths = {
      shader_path("coloured"),        shader_path("textured"),
      shader_path("textured_oxygen"), shader_path("water"),
      shader_path("player"),          shader_path("enemy"), 
      shader_path("collision_mesh")};
  std::array<GLuint, geometry_count> vertex_buffers;
  std::array<GLuint, geometry_count> index_buffers;
  std::array<Mesh, geometry_count>   meshes;

  public:
  // Initialize the window
  bool init(GLFWwindow* window);

  template <class T>
  void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices,
                     std::vector<uint16_t> indices);

  void initializeGlTextures();

  void initializeGlEffects();

  void  initializeGlMeshes();
  Mesh& getMesh(GEOMETRY_BUFFER_ID id) {
    return meshes[(int)id];
  };

  void initializeGlGeometryBuffers();

  void initializeGlCursor();

  // Initialize the screen texture used as intermediate render target
  // The draw loop first renders to this texture, then it is used for the wind
  // shader
  bool initScreenTexture();

  // Destroy resources associated to one or all entities created by the system
  ~RenderSystem();

  // Draw all entities
  void draw();

  mat3 createProjectionMatrix();

  private:
  // Internal drawing functions for each entity type
  void drawTexturedMesh(Entity entity, const mat3& projection);
  // void drawTexturedMeshTemp(Entity entity, const mat3& projection);
  void drawToScreen();

  // Window handle
  GLFWwindow* window;

  // Screen texture handles
  GLuint frame_buffer;
  GLuint off_screen_render_buffer_color;
  GLuint off_screen_render_buffer_depth;

  Entity screen_state_entity;
};

extern bool paused;

extern Entity player;
extern Entity player_weapon;

bool loadEffectFromFile(const std::string& vs_path, const std::string& fs_path,
                        GLuint& out_program);
