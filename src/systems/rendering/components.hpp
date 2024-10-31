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
  PLAYER               = 0,
  GUN                  = PLAYER + 1,
  HARPOON              = GUN + 1,
  PLAYER_OXYGEN_TANK   = HARPOON + 1,
  PLAYER_OXYGEN_BAR    = PLAYER_OXYGEN_TANK + 1,
  ENEMY_BACKGROUND_BAR = PLAYER_OXYGEN_BAR + 1,
  ENEMY_OXYGEN_BAR = ENEMY_BACKGROUND_BAR + 1,
  EMOTE_EXCLAMATION = ENEMY_OXYGEN_BAR + 1,
  EMOTE_QUESTION = EMOTE_EXCLAMATION + 1,
  WALL = EMOTE_QUESTION + 1,
  JELLY = WALL + 1,
  FISH0 = JELLY + 1,
  FISH1 = FISH0 + 1,
  FISH2 = FISH1 + 1,
  FISH3 = FISH2 + 1,
  FISH4 = FISH3 + 1,
  SHARK = FISH4 + 1,
  KRAB = SHARK + 1,
  BREAKABLE_CRATE = KRAB + 1,
  OXYGEN_CANISTER = BREAKABLE_CRATE + 1,
  GEYSER = OXYGEN_CANISTER + 1,
  TEXTURE_COUNT = GEYSER + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
  COLOURED        = 0,
  TEXTURED        = COLOURED + 1,
  TEXTURED_OXYGEN = TEXTURED + 1,
  WATER           = TEXTURED_OXYGEN + 1,
  EFFECT_COUNT    = WATER + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
  SPRITE          = 0,
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
