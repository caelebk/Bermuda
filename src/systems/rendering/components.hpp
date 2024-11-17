#pragma once
#include <vector>

#include "../ext/stb_image/stb_image.h"
#include "common.hpp"
#include "misc.hpp"
// Sets the brightness of the screen
struct ScreenState {
  float darken_screen_factor = -1;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl &
// salmon.vs.glsl)
struct ColoredVertex {
  vec3 position;
  vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex {
  vec3 position;
  vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh {
  static bool                loadFromOBJFile(std::string                 obj_path,
                                             std::vector<ColoredVertex>& out_vertices,
                                             std::vector<uint16_t>&      out_vertex_indices,
                                             vec2&                       out_size);
  vec2                       original_size = {1, 1};
  std::vector<ColoredVertex> vertices;
  std::vector<uint16_t>      vertex_indices;
};

// font character structure
struct Character {
  unsigned int TextureID;  // ID handle of the glyph texture
  glm::ivec2   Size;       // Size of glyph
  glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
  unsigned int Advance;    // Offset to advance to next glyph
  char         character;
};

struct TextRequest {
  std::string text;
  float       textScale;
};

enum class SOUND_ASSET_ID {
  PLAYER_OXYGEN_BLAST = 0,
  PLAYER_GLIDE = PLAYER_OXYGEN_BLAST + 1,
  PLAYER_SLOW_HEART = PLAYER_GLIDE + 1,
  PLAYER_FAST_HEART = PLAYER_SLOW_HEART + 1,
  PLAYER_HURT = PLAYER_FAST_HEART + 1,
  PLAYER_DEATH = PLAYER_HURT + 1,
  PLAYER_FLATLINE = PLAYER_DEATH + 1,
  PLAYER_EAT = PLAYER_FLATLINE + 1,
  PLAYER_DEPLETE = PLAYER_EAT + 1,
  PLAYER_EMPTY_GUN = PLAYER_DEPLETE + 1,
  PROJECTILE_CONCUSSIVE = PLAYER_EMPTY_GUN + 1,
  PROJECTILE_TORPEDO = PROJECTILE_CONCUSSIVE + 1,
  EXPLOSION = PROJECTILE_TORPEDO + 1,
  PROJECTILE_SHRIMP = EXPLOSION + 1,
  ENEMY_DEATH = PROJECTILE_SHRIMP + 1,
  DOOR = ENEMY_DEATH + 1,
  KEY = DOOR + 1,
  PROJECTILE_NET = KEY + 1,
  CRATE_DEATH = PROJECTILE_NET + 1,
  CRATE_HIT = CRATE_DEATH + 1,
  METAL_CRATE_DEATH = CRATE_HIT + 1,
  METAL_CRATE_HIT = METAL_CRATE_DEATH + 1,
  LOBSTER_SHIELD = METAL_CRATE_HIT + 1,
  SOUND_COUNT = LOBSTER_SHIELD + 1
};

const int sound_count = (int)SOUND_ASSET_ID::SOUND_COUNT;

enum class MUSIC_ASSET_ID {
  MUSIC1 = 0,
  MUSIC_COUNT = MUSIC1 + 1
};

const int music_count = (int)MUSIC_ASSET_ID::MUSIC_COUNT;
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
 * the renderer encapsulates all asset data and the game logic is agnostic to
 * it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
  PAUSE                = 0,
  HARPOON_COUNTER      = PAUSE + 1,
  INVENTORY            = HARPOON_COUNTER + 1,
  COMMUNUCATIONS       = INVENTORY + 1,
  CURSOR               = COMMUNUCATIONS + 1,
  PLAYER1              = CURSOR + 1,
  PLAYER2              = PLAYER1 + 1,
  PLAYER3              = PLAYER2 + 1,
  PLAYER_DASH          = PLAYER3 + 1,
  HARPOON_GUN          = PLAYER_DASH + 1,
  HARPOON              = HARPOON_GUN + 1,
  NET_GUN              = HARPOON + 1,
  NET                  = NET_GUN + 1,
  CONCUSSIVE_GUN       = NET + 1,
  CONCUSSIVE           = CONCUSSIVE_GUN + 1,
  TORPEDO_GUN          = CONCUSSIVE + 1,
  TORPEDO              = TORPEDO_GUN + 1,
  SHRIMP_GUN           = TORPEDO + 1,
  SHRIMP               = SHRIMP_GUN + 1,  // TODO: shrimp gun projectile needed
  RED_KEY              = SHRIMP + 1,
  BLUE_KEY             = RED_KEY + 1,
  YELLOW_KEY           = BLUE_KEY + 1,
  PLAYER_OXYGEN_TANK   = YELLOW_KEY + 1,
  PLAYER_OXYGEN_BAR    = PLAYER_OXYGEN_TANK + 1,
  ENEMY_BACKGROUND_BAR = PLAYER_OXYGEN_BAR + 1,
  ENEMY_OXYGEN_BAR     = ENEMY_BACKGROUND_BAR + 1,
  EMOTE_EXCLAMATION    = ENEMY_OXYGEN_BAR + 1,
  EMOTE_QUESTION       = EMOTE_EXCLAMATION + 1,
  WALL                 = EMOTE_QUESTION + 1,
  TUTORIAL_FLOOR       = WALL + 1,
  FLOOR                = TUTORIAL_FLOOR + 1,
  LOCKED_DOOR_H        = FLOOR + 1,
  LOCKED_DOOR_V        = LOCKED_DOOR_H + 1,
  LOCKED_YELLOW_H      = LOCKED_DOOR_V + 1,
  LOCKED_YELLOW_V      = LOCKED_YELLOW_H + 1,
  LOCKED_BLUE_H        = LOCKED_YELLOW_V + 1,
  LOCKED_BLUE_V        = LOCKED_BLUE_H + 1,
  LOCKED_RED_H         = LOCKED_BLUE_V + 1,
  LOCKED_RED_V         = LOCKED_RED_H + 1,
  DOORWAY_H            = LOCKED_RED_V + 1,
  DOORWAY_V            = DOORWAY_H + 1,
  JELLY                = DOORWAY_V + 1,
  FISH0                = JELLY + 1,
  FISH1                = FISH0 + 1,
  FISH2                = FISH1 + 1,
  FISH3                = FISH2 + 1,
  FISH4                = FISH3 + 1,
  SHARK                = FISH4 + 1,
  KRAB                 = SHARK + 1,
  URCHIN               = KRAB + 1,
  URCHIN_NEEDLE        = URCHIN + 1,
  KRAB_BOSS            = URCHIN_NEEDLE + 1,
  KRAB_BOSS_CHASE      = KRAB_BOSS + 1,
  SHARKMAN0            = KRAB_BOSS_CHASE + 1,
  SHARKMAN1            = SHARKMAN0 + 1,
  SHARKMAN2            = SHARKMAN1 + 1,
  SHARKMAN3            = SHARKMAN2 + 1,
  SHARKMAN4            = SHARKMAN3 + 1,
  SHARKMAN5            = SHARKMAN4 + 1,
  SHARKMAN6            = SHARKMAN5 + 1,
  SHARKMAN7            = SHARKMAN6 + 1,
  LOBSTER              = SHARKMAN7 + 1,
  LOBSTER_BLOCK        = LOBSTER + 1,
  LOBSTER_RAM          = LOBSTER_BLOCK + 1,
  BREAKABLE_CRATE      = LOBSTER_RAM + 1,
  METAL_CRATE          = BREAKABLE_CRATE + 1,
  OXYGEN_CANISTER      = METAL_CRATE + 1,
  GEYSER               = OXYGEN_CANISTER + 1,
  GEYSER_BUBBLE        = GEYSER + 1,
  TEXTURE_COUNT        = GEYSER_BUBBLE + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
  COLOURED        = 0,
  TEXTURED        = COLOURED + 1,
  TEXTURED_OXYGEN = TEXTURED + 1,
  WATER           = TEXTURED_OXYGEN + 1,
  PLAYER          = WATER + 1,
  ENEMY           = PLAYER + 1,
  COLLISION_MESH  = ENEMY + 1,
  EFFECT_COUNT    = COLLISION_MESH + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
  PLAYER          = 0,
  SPRITE          = PLAYER + 1,
  DEBUG_LINE      = SPRITE + 1,
  SCREEN_TRIANGLE = DEBUG_LINE + 1,
  GEOMETRY_COUNT  = SCREEN_TRIANGLE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
  TEXTURE_ASSET_ID   used_texture  = TEXTURE_ASSET_ID::TEXTURE_COUNT;
  EFFECT_ASSET_ID    used_effect   = EFFECT_ASSET_ID::EFFECT_COUNT;
  GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
};
