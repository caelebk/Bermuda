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
      std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::PLAYER,
                                                 mesh_path("player.obj"))};

  // Make sure these paths remain in sync with the associated enumerators.
  const std::array<std::string, texture_count> texture_paths = {
      textures_path("overlays/intro_overlay.png"),
      textures_path("overlays/start_overlay.png"),
      textures_path("overlays/pause_overlay.png"),
      textures_path("overlays/krab_overlay.png"),
      textures_path("overlays/sharkman_overlay.png"),
      textures_path("overlays/cthulhu_overlay.png"),
      textures_path("overlays/death_overlay.png"),
      textures_path("overlays/end_overlay.png"),
      textures_path("player/infinity.png"),
      textures_path("player/inventory.png"),
      textures_path("player/communications.png"),
      textures_path("cursors/crosshair_cursor.png"),  // source: kenney.nl
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
      textures_path("player/explosion.png"),
      textures_path("player/shrimp_gun.png"),
      textures_path(
          "player/shrimp.png"),  // inspired by:
                                 // https://www.artstation.com/artwork/g0Q3zG
      textures_path("consumables/red_key.png"),
      textures_path("consumables/blue_key.png"),
      textures_path("consumables/yellow_key.png"),
      textures_path("oxygen_and_health/player_oxygen_tank.png"),
      textures_path("oxygen_and_health/player_oxygen_bar.png"),
      textures_path("oxygen_and_health/enemy_background_bar.png"),
      textures_path("oxygen_and_health/enemy_oxygen_bar.png"),
      textures_path("enemy/exclamation.png"),
      textures_path("enemy/question.png"),
      textures_path("map_non_interactable/wall.png"),
      textures_path(
          "map_non_interactable/tutorial_floor.png"),  // partial source:
                                                       // Vecteezy.com
      textures_path("map_non_interactable/floor.png"),
      textures_path(
          "map_non_interactable/anchor.png"),  // source:
                                               // https://create.vista.com/vectors/icon-with-iron-anchor/
      textures_path(
          "map_non_interactable/bag.png"),  // source:
                                            // https://www.gettyimages.ca/detail/illustration/plastic-bag-royalty-free-illustration/451573859?adppopup=true
      textures_path(
          "map_non_interactable/bones.png"),  // source:
                                              // https://www.cleanpng.com/png-pile-of-bones-skeleton-bones-pile-scattered-pile-o-8142370/7.html
      textures_path(
          "map_non_interactable/bones1.png"),  // source:
                                               // https://www.freepik.com/premium-ai-image/cartoon-bone_334442552.htm
      textures_path(
          "map_non_interactable/bottle.png"),  // source:
                                               // https://pngtree.com/freepng/plastic-bottle-cartoon-illustration_7262953.html
      textures_path("map_non_interactable/coral1.png"),
      textures_path("map_non_interactable/coral2.png"),
      textures_path("map_non_interactable/seaweed1.png"),
      textures_path(
          "map_non_interactable/shell1.png"),  // shells source:
                                               // https://www.clipartmax.com/middle/m2i8d3d3N4d3A0N4_seashell-pin-png-cartoon-seashell-png/
      textures_path("map_non_interactable/shell2.png"),
      textures_path("map_non_interactable/shell3.png"),
      textures_path("map_non_interactable/tire.png"),
      textures_path(
          "map_non_interactable/seaweed1.png"),  // source:
                                                 // https://pngtree.com/free-foods-clipart/kelp
      textures_path("map_non_interactable/seaweed1.png"),

      textures_path("map_interactable/locked_door_h.png"),
      textures_path("map_interactable/locked_door_v.png"),
      textures_path("map_interactable/locked_yellow_h.png"),
      textures_path("map_interactable/locked_yellow_v.png"),
      textures_path("map_interactable/locked_blue_h.png"),
      textures_path("map_interactable/locked_blue_v.png"),
      textures_path("map_interactable/locked_red_h.png"),
      textures_path("map_interactable/locked_red_v.png"),
      textures_path(
          "map_interactable/boss_doorway_h.png"),  // source:
                                                   // https://frodoundead.itch.io/skull-bones
      textures_path("map_interactable/boss_doorway_v.png"),  // ibidem
      textures_path("map_interactable/doorway_h.png"),
      textures_path("map_interactable/doorway_v.png"),
      textures_path("enemy/jellyfish.png"),  // source: Vecteezy.com
      textures_path(
          "enemy/fish0.png"),  // all fishes source:
                               // https://snowdingo.itch.io/pixel-fishes
      textures_path("enemy/fish1.png"),
      textures_path("enemy/fish2.png"),
      textures_path("enemy/fish3.png"),
      textures_path("enemy/fish4.png"),
      textures_path("enemy/shark.png"),
      textures_path("enemy/krab.png"),  // source: Vecteezy.com
      textures_path("enemy/urchin.png"),
      textures_path("enemy/urchin_needle.png"),
      textures_path("enemy/turtle.png"),
      textures_path(
          "enemy/seahorse.png"),  // source:
                                  // https://www.freepik.com/premium-vector/pixel-art-illustration-seahorse-pixelated-seahorse-ocean-seahorse-icon-pixelated_80858279.htm
      textures_path(
          "enemy/seahorse_bullet.png"),  // source:
                                         // https://www.pngitem.com/middle/bwmTmm_8-bit-bullet-png-transparent-png/
      textures_path(
          "enemy/siren.png"),  // inspired by
                               // https://www.tumblr.com/bakubabes-and-ramble
      textures_path("enemy/siren_heal.png"),
      textures_path(
          "enemy/krab_boss.png"),  // krabs and krab boss source: Vecteezy.com
      textures_path("enemy/krab_boss_chase.png"),
      textures_path(
          "enemy/sharkman/sharkman0.png"),  // sharkman source:
                                            // https://www.deviantart.com/h20dynamo/art/Shark-Man-Pixel-Animation-722233181
      textures_path("enemy/sharkman/sharkman1.png"),
      textures_path("enemy/sharkman/sharkman2.png"),
      textures_path("enemy/sharkman/sharkman3.png"),
      textures_path("enemy/sharkman/sharkman4.png"),
      textures_path("enemy/sharkman/sharkman5.png"),
      textures_path("enemy/sharkman/sharkman6.png"),
      textures_path("enemy/sharkman/sharkman7.png"),
      textures_path("enemy/lobster/lobster.png"),
      textures_path("enemy/lobster/lobster_block.png"),
      textures_path("enemy/lobster/lobster_ram.png"),
      textures_path("enemy/cthulhu/cthulhu_normal.png"),  // source: seekpng.com
      textures_path("enemy/cthulhu/cthulhu_rage.png"),
      textures_path(
          "enemy/tentacle.png"),  // original source:
                                  // https://www.pixilart.com/draw/tentacle-0dbf4493af56288
      textures_path("enemy/fireball.png"),  // extracted from cthulhu source
      textures_path(
          "enemy/rage_proj.png"),  // source:
                                   // https://www.deviantart.com/venjix5/art/Green-Energy-Ball-16-alt-831898817
      textures_path(
          "enemy/shockwave.png"),  // source:
                                   // https://www.unrealengine.com/marketplace/en-US/product/shockwave-pack
      textures_path(
          "map_interactable/breakable_crate.png"),  // source:
                                                    // https://www.freepik.com/premium-vector/cartoon-crate-icon-isolated-vector-graphic-illustration-pixel-art-icon-game-assets-bi-sprite_26690845.htm
      textures_path(
          "map_interactable/metal_crate.png"),  // source:
                                                // https://static.wikia.nocookie.net/portalworldsgame/images/a/a7/Metal_Plate.png/revision/latest?cb=20190611100746
      textures_path(
          "consumables/oxygen_canister.png"),  // source:
                                               // https://opensea.io/assets/ethereum/0x9680223f7069203e361f55fefc89b7c1a952cdcc/380
      textures_path("map_interactable/geyser.png"),
      textures_path("map_interactable/geyser_bubble.png"),
      textures_path("map_interactable/pressure_plate/pressure_on.png"),
      textures_path("map_interactable/pressure_plate/pressure_off.png"),
      textures_path("map_interactable/rock.png"),
  };

  std::array<GLuint, effect_count> effects;
  // Make sure these paths remain in sync with the associated enumerators.
  const std::array<std::string, effect_count> effect_paths = {
      shader_path("coloured"),        shader_path("textured"),
      shader_path("textured_oxygen"), shader_path("water"),
      shader_path("player"),          shader_path("enemy"),
      shader_path("ambient"),         shader_path("collision_mesh"),
      shader_path("communications")};
  std::array<GLuint, geometry_count> vertex_buffers;
  std::array<GLuint, geometry_count> index_buffers;
  std::array<Mesh, geometry_count>   meshes;

  public:
  // Initialize the window
  bool init(GLFWwindow* window);

  template <class T>
  void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices,
                     std::vector<uint16_t> indices);

  bool fontInit();

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

  void processTextRequest(Entity& entity);
  mat3 createProjectionMatrix();

  private:
  // Internal drawing functions for each entity type
  void drawTexturedMesh(Entity entity, const mat3& projection);
  // void drawTexturedMeshTemp(Entity entity, const mat3& projection);
  void drawToScreen();
  void renderText(std::string text, float x, float y, float scale,
                  const glm::vec3& color, const glm::mat4& trans);

  // Window handle
  GLFWwindow* window;

  // Screen texture handles
  GLuint frame_buffer;
  GLuint off_screen_render_buffer_color;
  GLuint off_screen_render_buffer_depth;

  // render system vao
  GLuint vao;

  // font-specific elements
  std::map<char, Character> fontCharacters;
  GLuint                    font_shaderProgram;
  GLuint                    font_VAO;
  GLuint                    font_VBO;

  Entity screen_state_entity;
};

extern bool is_intro;
extern bool is_start;
extern bool is_paused;
extern bool is_krab_cutscene;
extern bool is_sharkman_cutscene;
extern bool is_cthulhu_cutscene;
extern bool is_death;
extern bool is_end;
extern bool room_transitioning;

extern Entity player;
extern Entity player_weapon;

bool loadEffectFromFile(const std::string& vs_path, const std::string& fs_path,
                        GLuint& out_program);

std::string readShaderFile(const std::string& filename);
