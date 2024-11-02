#include "map_factories.hpp"

#include <iostream>

#include "collision_system.hpp"
#include "enemy_factories.hpp"
#include "oxygen_system.hpp"
#include "physics_system.hpp"
#include "random.hpp"

/////////////////////////////////////////////////////////////////
// Util
/////////////////////////////////////////////////////////////////
/**
 * @brief Checks whether or not the spawn is valid or invalid based on spawn
 * collisons The entity should already have the position attached
 *
 * @param entity - enemy to check
 * @return true if valid, false otherwise
 */
static bool checkSpawnCollisions(Entity entity) {
  if (!registry.positions.has(entity)) {
    return false;
  }
  const Position& entityPos = registry.positions.get(entity);

  // Entities can't spawn in the player
  for (Entity player : registry.players.entities) {
    if (!registry.positions.has(player)) {
      continue;
    }
    const Position player_pos = registry.positions.get(player);
    if (box_collides(entityPos, player_pos)) {
      return false;
    }
  }

  // Entities can't spawn in walls
  for (Entity wall : registry.activeWalls.entities) {
    if (!registry.positions.has(wall)) {
      continue;
    }
    const Position wallPos = registry.positions.get(wall);
    if (box_collides(entityPos, wallPos)) {
      return false;
    }
  }

  // Entities can't spawn in interactables
  for (Entity interactable : registry.interactable.entities) {
    if (!registry.positions.has(interactable)) {
      continue;
    }
    const Position interactablePos = registry.positions.get(interactable);
    if (box_collides(entityPos, interactablePos)) {
      return false;
    }
  }

  // Entities can't spawn in enemies
  for (Entity deadlys : registry.deadlys.entities) {
    if (!registry.positions.has(deadlys)) {
      continue;
    }
    const Position deadlyPos = registry.positions.get(deadlys);
    if (box_collides(entityPos, deadlyPos)) {
      return false;
    }
  }

  return true;
}
/////////////////////////////////////////////////////////////////
// Geyser
/////////////////////////////////////////////////////////////////
/**
 * @brief creates a geyser tank at a specific position
 *
 * @param renderer
 * @param position
 * @return
 */
Entity createGeyserPos(RenderSystem* renderer, vec2 position) {
  // Reserve an entity
  auto entity = Entity();

  auto& pos    = registry.positions.emplace(entity);
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = GEYSER_SCALE_FACTOR * GEYSER_BOUNDING_BOX;

  if (!checkSpawnCollisions(entity)) {
    // returns invalid entity, since id's start from 1
    registry.remove_all_components_of(entity);
    return Entity(0);
  }

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make consumable
  registry.interactable.emplace(entity);

  // Add stats
  auto& refill        = registry.oxygenModifiers.emplace(entity);
  refill.amount       = GEYSER_QTY;
  auto& modifyOxygenCd      = registry.modifyOxygenCd.emplace(entity);
  modifyOxygenCd.default_cd = GEYSER_RATE_MS;

  // physics and pos

  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::GEYSER, EFFECT_ASSET_ID::TEXTURED,
               GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}
/////////////////////////////////////////////////////////////////
// crate
/////////////////////////////////////////////////////////////////
/**
 * @brief creates a breakable crate at a specific position
 *
 * @param renderer
 * @param position
 * @return
 */

Entity createCratePos(RenderSystem* renderer, vec2 position) {
  // Reserve an entity
  auto entity = Entity();
  // physics and pos
  auto& pos    = registry.positions.emplace(entity);
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = CRATE_SCALE_FACTOR * CRATE_BOUNDING_BOX;
  pos.scale *= 4.f;
  if (!checkSpawnCollisions(entity)) {
    // returns invalid entity, since id's start from 1
    registry.remove_all_components_of(entity);
    return Entity(0);
  }

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // reuse wall code
  registry.activeWalls.emplace(entity);
  registry.deadlys.emplace(entity);

  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::BREAKABLE_CRATE, EFFECT_ASSET_ID::TEXTURED,
               GEOMETRY_BUFFER_ID::SPRITE});

  createDefaultHealthbar(renderer,entity, CRATE_HEALTH, CRATE_HEALTH_SCALE, CRATE_HEALTH_BAR_SCALE, CRATE_HEALTH_BOUNDING_BOX);

  // assign drops
  if (randomSuccess(CRATE_DROP_CHANCE_0)) {
    Drop &drop = registry.drops.emplace(entity);
    drop.dropFn = CRATE_DROP_0;
    return entity;
  }

  return entity;
}

