#include "map_factories.hpp"

#include <cstdio>
#include <iostream>

#include "collision_system.hpp"
#include "enemy_factories.hpp"
#include "environment.hpp"
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

  for (Entity door : registry.activeDoors.entities) {
    if (!registry.positions.has(door)) {
      continue;
    }
    const Position doorPos = registry.positions.get(door);
    vec2 dist_vec = entityPos.position - doorPos.position;
    float dist = sqrt(dot(dist_vec, dist_vec));
    if (dist < DOOR_SPAWN_RADIUS) {
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
Entity createGeyserPos(RenderSystem* renderer, vec2 position, bool checkCollisions) {
  // Reserve an entity
  auto entity = Entity();

  auto& pos    = registry.positions.emplace(entity);
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = GEYSER_SCALE_FACTOR * GEYSER_BOUNDING_BOX;

  if (checkCollisions && !checkSpawnCollisions(entity)) {
    // returns invalid entity, since id's start from 1
    registry.remove_all_components_of(entity);
    return Entity(0);
  }

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make consumable
  Interactable &i = registry.interactable.emplace(entity);
  i.respawnFn = respawnGeyser;

  // designate this as a geyser; facilitates room transitions.
  registry.geysers.emplace(entity);

  // Add stats
  auto& refill              = registry.oxygenModifiers.emplace(entity);
  refill.amount             = GEYSER_QTY;
  auto& modifyOxygenCd      = registry.modifyOxygenCd.emplace(entity);
  modifyOxygenCd.default_cd = GEYSER_RATE_MS;

  // physics and pos

  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::GEYSER, EFFECT_ASSET_ID::TEXTURED,
               GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}

/**
 * @brief Respawns a Geyser based on it's entity state
 *
 * @param renderer 
 * @param es 
 * @return 
 */
Entity respawnGeyser(RenderSystem *renderer, EntityState es) {
  Entity entity = createGeyserPos(renderer, es.position.position, false);
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

Entity createCratePos(RenderSystem* renderer, vec2 position, bool checkCollisions) {
  // Reserve an entity
  auto entity = Entity();
  // physics and pos
  auto& pos    = registry.positions.emplace(entity);
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = CRATE_SCALE_FACTOR * CRATE_BOUNDING_BOX;
  pos.scale *= 4.f;
  if (checkCollisions && !checkSpawnCollisions(entity)) {
    // returns invalid entity, since id's start from 1
    registry.remove_all_components_of(entity);
    return Entity(0);
  }

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // reuse wall code
  registry.activeWalls.emplace(entity);
  Breakable &b = registry.breakables.emplace(entity);
  b.respawnFn = respawnCrate;

  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::BREAKABLE_CRATE, EFFECT_ASSET_ID::TEXTURED,
               GEOMETRY_BUFFER_ID::SPRITE});

  createDefaultHealthbar(renderer, entity, CRATE_HEALTH, CRATE_HEALTH_SCALE,
                         CRATE_HEALTH_BAR_SCALE, CRATE_HEALTH_BOUNDING_BOX);

  // assign drops
  if (randomSuccess(CRATE_DROP_CHANCE_0)) {
    Drop& drop  = registry.drops.emplace(entity);
    drop.dropFn = CRATE_DROP_0;
    return entity;
  }

  return entity;
}

/**
 * @brief Respawns a Crate based on it's entity state
 *
 * @param renderer 
 * @param es 
 * @return 
 */
Entity respawnCrate(RenderSystem *renderer, EntityState es) {
  Entity entity = createCratePos(renderer, es.position.position, false);

  // respawn failed, ignore
  if ((unsigned int) entity == 0) {
    return Entity(0);
  }

  // Restore State
  Position &pos = registry.positions.get(entity);
  pos.angle = es.position.angle;
  pos.scale = es.position.scale;
  pos.originalScale = es.position.originalScale;

  Oxygen &o = registry.oxygen.get(entity);
  float diff = es.oxygen - o.level;

  // This will also update the health bar
  if (diff < 0) {
    modifyOxygenAmount(entity, diff);
  }

  return entity;
}


