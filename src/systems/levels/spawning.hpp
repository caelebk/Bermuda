#pragma once

#include "level.hpp"
#include "random.hpp"
#include "render_system.hpp"
#include <functional>
#include <glm/ext/vector_float2.hpp>
#include <initializer_list>
#include <random>
#include <vector>

#include "space.hpp"

/**
 * @brief Takes in a config macro, and spawns enemies in random locations in the
 * Space
 *
 * @param funcs - config, as a initalizer list of factory functions
 * @param space_builder - SpaceBuilder, room or hallway
 * @param renderer
 */
template <typename T>
void execute_config_rand(
    const std::initializer_list<std::function<Entity(RenderSystem *r, vec2 p, bool b)>>
        &funcs,
    SpaceBuilder<T> &space_builder, RenderSystem *renderer) {
  for (const auto &func : funcs) {
    vec2 loc;
    do {
      loc = space_builder.get_random_position();
    } while ((unsigned int)func(renderer, loc, true) ==
             0); // Call each function until spawn is successful
  }
}

/**
 * @brief Takes in a config macro, and spawns enemies in random locations in the
 * Space with that % chance
 *
 * @param funcs - config, as a initalizer list of factory functions
 * @param space_builder - SpaceBuilder, room or hallway
 * @param chance - float [0,1] describing the % chance of each occuring
 */
template <typename T>
void execute_config_rand_chance(
    const std::initializer_list<std::function<Entity(RenderSystem *r, vec2 p, bool b)>>
        &funcs,
    SpaceBuilder<T> &space_builder, RenderSystem *renderer, float chance) {
  for (const auto &func : funcs) {
    if (randomSuccess(chance)) {
      vec2 loc;
      do {
        loc = space_builder.get_random_position();
      } while ((unsigned int)func(renderer, loc, true) == 0); // Call each function until spawn is successful
    }
  }
}

/**
 * @brief Takes in a config macro with pre-defined positioned entities, and
 * executes them
 *
 * @param funcs - functions that return an entity at a specific location
 */
void execute_config_fixed(const std::vector<std::function<void()>> &funcs);

/**
 * @brief Takes in a config macro with pre-defined positioned entities, and
 * executes them
 *
 * @param funcs - functions that return an entity at a specific location
 */
void execute_config_fixed_rand(float chance,
                               const std::vector<std::function<void()>> &funcs);

/**
 * @brief removes all drops and enemies from the level
 *
 * @return true if success
 */
bool remove_all_entities();
