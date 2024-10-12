#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem
{
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;

  // Stored cursor asset
  GLFWimage cursorImageData;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	const std::vector<std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
		{
			// specify meshes of other assets here
	};

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, texture_count> texture_paths = {
		textures_path("player/player.png"),
		textures_path("player/harpoon_gun.png"),
		textures_path("player/harpoon.png"),
		textures_path("oxygen_and_health/player_oxygen_tank.png"),
		textures_path("oxygen_and_health/player_oxygen_bar.png"),
		textures_path("oxygen_and_health/enemy_background_bar.png"),
		textures_path("oxygen_and_health/enemy_oxygen_bar.png"),
		textures_path("map_non_interactable/solid_colour_wall.png"),
		textures_path("enemy/jellyfish.png"),
		textures_path("enemy/fish.png"),
      		textures_path("consumables/oxygen_tank.png"),
      		textures_path("map_interactable/geyser.png"),
	};

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"),
		shader_path("textured"),
		shader_path("textured_oxygen"),
		shader_path("water")};
  std::array<GLuint, geometry_count> vertex_buffers;
  std::array<GLuint, geometry_count> index_buffers;
  std::array<Mesh, geometry_count> meshes;

public:
  // Initialize the window
  bool init(GLFWwindow *window);

  template <class T>
  void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices,
                     std::vector<uint16_t> indices);

  void initializeGlTextures();

  void initializeGlEffects();

	void initializeGlMeshes();
	Mesh &getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };
  void initializeGlMeshes();
};

bool loadEffectFromFile(const std::string &vs_path, const std::string &fs_path,
                        GLuint &out_program);
#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem
{
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;

  // Stored cursor asset
  GLFWimage cursorImageData;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	const std::vector<std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
		{
			// specify meshes of other assets here
	};

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, texture_count> texture_paths = {
		textures_path("player/player.png"),
		textures_path("player/harpoon_gun.png"),
		textures_path("player/harpoon.png"),
		textures_path("oxygen_and_health/player_oxygen_tank.png"),
		textures_path("oxygen_and_health/player_oxygen_bar.png"),
		textures_path("oxygen_and_health/enemy_background_bar.png"),
		textures_path("oxygen_and_health/enemy_oxygen_bar.png"),
		textures_path("map_non_interactable/solid_colour_wall.png"),
		textures_path("enemy/jellyfish.png"),
		textures_path("enemy/fish.png"),
	};

