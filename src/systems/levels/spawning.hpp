#pragma once

#include "level_system.hpp"
#include "ai.hpp"
#include "enemy_factories.hpp"
#include "random.hpp"
#include "render_system.hpp"
#include <cstdio>
#include <functional>
#include <glm/ext/vector_float2.hpp>
#include <random>
#include <vector>

#include "render_system.hpp"
#include "room_builder.hpp"

#define MAX_SPAWN_ATTEMPTS 64
#define MIN_PACK_SHAPE_SIZE 300.f
#define MAX_PACK_SHAPE_SIZE 500.f

/**
 * @brief Takes in a config macro, and spawns enemies in random locations in the
 * Space
 *
 * @param funcs - config, as a initalizer list of factory functions
 * @param room_builder - room
 * @param renderer
 */
void execute_config_rand(
    const std::vector<std::function<Entity(RenderSystem *r, vec2 p, bool b)>>
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
    const std::vector<std::function<Entity(RenderSystem *r, vec2 p, bool b)>>
        &funcs,
    RoomBuilder &room_builder, RenderSystem *renderer, float chance);

/////////////////////////////////////////////////////////////////
// group spawning
/////////////////////////////////////////////////////////////////
/**
 * @brief spawnns a group of enemies within a particular area
 *
 * @param spawnFn enemy factory function
 * @param pack_size the number of enemies to create > 0
 */
void execute_pack_spawning(std::function<Entity(RenderSystem *r, vec2 p, bool b)> spawnFn, RoomBuilder &room_builder, RenderSystem *renderer, int pack_size); 

void execute_pack_spawning_area_size(
    std::function<Entity(RenderSystem* r, vec2 p, bool b)> spawnFn,
    RoomBuilder& room_builder, RenderSystem* renderer, int pack_size,
    float width, float height);


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
