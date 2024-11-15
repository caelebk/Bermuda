#pragma once

#include "level_system.hpp"
#include "random.hpp"
#include "render_system.hpp"
#include <functional>
#include <glm/ext/vector_float2.hpp>
#include <initializer_list>
#include <random>
#include <vector>

#include "room_builder.hpp"

/**
 * @brief Takes in a config macro, and spawns enemies in random locations in the
 * Space
 *
 * @param funcs - config, as a initalizer list of factory functions
 * @param room_builder - room
 * @param renderer
 */
void execute_config_rand(
    const std::initializer_list<std::function<Entity(RenderSystem *r, vec2 p, bool b)>>
        &funcs,
    RoomBuilder &room_builder, RenderSystem *renderer);

/**
 * @brief Takes in a config macro, and spawns enemies in random locations in the
 * Space with that % chance
 *
 * @param funcs - config, as a initalizer list of factory functions
 * @param space_builder - SpaceBuilder, room or hallway
 * @param chance - float [0,1] describing the % chance of each occuring
 */
void execute_config_rand_chance(
    const std::initializer_list<std::function<Entity(RenderSystem *r, vec2 p, bool b)>>
        &funcs,
    RoomBuilder &room_builder, RenderSystem *renderer, float chance);

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
