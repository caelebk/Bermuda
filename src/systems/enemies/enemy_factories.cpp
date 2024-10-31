#include "enemy_factories.hpp"

#include <iostream>

#include "collision_system.hpp"
#include "components.hpp"
#include "oxygen_system.hpp"
#include "physics.hpp"
#include "random.hpp"
#include "space.hpp"
#include "tiny_ecs_registry.hpp"

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
  const Position& enemyPos = registry.positions.get(entity);

  // Entities can't spawn in the player
  for (Entity player : registry.players.entities) {
    if (!registry.positions.has(player)) {
      continue;
    }
    const Position player_pos = registry.positions.get(player);
    if (box_collides(enemyPos, player_pos)) {
      return false;
    }
  }

  // Entities can't spawn in walls
  for (Entity wall : registry.activeWalls.entities) {
    if (!registry.positions.has(wall)) {
      continue;
    }
    const Position wallPos = registry.positions.get(wall);
    if (box_collides(enemyPos, wallPos)) {
      return false;
    }
  }

  // Entities can't spawn in other enemies
  for (Entity deadly : registry.deadlys.entities) {
    if (!registry.positions.has(deadly)) {
      continue;
    }
    const Position deadlyPos = registry.positions.get(deadly);
    if (box_collides(enemyPos, deadlyPos)) {
      return false;
    }
  }

  // Entities can't spawn in interactables
  for (Entity interactable : registry.interactable.entities) {
    if (!registry.positions.has(interactable)) {
      continue;
    }
    const Position interactablePos = registry.positions.get(interactable);
    if (box_collides(enemyPos, interactablePos)) {
      return false;
    }
  }

  return true;
}

/////////////////////////////////////////////////////////////////
// Jellyfish
/////////////////////////////////////////////////////////////////
/**
 * @brief creates a fish at a specific position
 *
 * @param renderer
 * @param position
 * @return the entity if successful, 0 otherwise
 */
Entity createJellyPos(RenderSystem* renderer, vec2 position) {
  // Reserve an entity
  auto entity = Entity();

  auto& pos    = registry.positions.emplace(entity);
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = JELLY_SCALE_FACTOR * JELLY_BOUNDING_BOX;

  if (!checkSpawnCollisions(entity)) {
    // returns invalid entity, since id's start from 1
    registry.remove_all_components_of(entity);
    return Entity(0);
  }

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make enemy
  registry.deadlys.emplace(entity);

  // Add stats
  auto& damage  = registry.oxygenModifiers.emplace(entity);
  damage.amount = JELLY_DAMAGE;

  auto& modifyOxygenCd      = registry.modifyOxygenCd.emplace(entity);
  modifyOxygenCd.default_cd = JELLY_ATK_SPD;

  // add abilities
  auto& stun    = registry.stuns.emplace(entity);
  stun.duration = JELLY_STUN_MS;

  // physics and pos

  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::JELLY, EFFECT_ASSET_ID::TEXTURED,
               GEOMETRY_BUFFER_ID::SPRITE});

  createDefaultHealthbar(renderer, entity, JELLY_HEALTH, JELLY_HEALTH_SCALE,
                         JELLY_HEALTH_BAR_SCALE, JELLY_HEALTH_BOUNDING_BOX);

  return entity;
}

/////////////////////////////////////////////////////////////////
// Fish
/////////////////////////////////////////////////////////////////
/**
 * @brief creates a fish at a specific position
 *
 * @param renderer
 * @param position
 * @return
 */
Entity createFishPos(RenderSystem* renderer, vec2 position) {
  // Reserve an entity
  auto entity = Entity();

  auto& pos    = registry.positions.emplace(entity);
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = FISH_SCALE_FACTOR * FISH_BOUNDING_BOX;
  if (!checkSpawnCollisions(entity)) {
    // returns invalid entity, since id's start from 1
    registry.remove_all_components_of(entity);
    return Entity(0);
  }

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make enemy and damage
  registry.deadlys.emplace(entity);
  auto& damage  = registry.oxygenModifiers.emplace(entity);
  damage.amount = FISH_DAMAGE;

  auto& modifyOxygenCd      = registry.modifyOxygenCd.emplace(entity);
  modifyOxygenCd.default_cd = FISH_ATK_SPD;

  // Initialize the position, scale, and physics components
  auto& motion        = registry.motions.emplace(entity);
  motion.velocity     = {-FISH_MS, 0};
  motion.acceleration = {0, 0};

  // ai
  auto& wander         = registry.wanders.emplace(entity);
  wander.active_dir_cd = 0;  // immediately picks a new direction
  wander.change_dir_cd = getRandInt(FISH_MIN_DIR_CD, FISH_MAX_DIR_CD);

  // TODO: add the room
  TEXTURE_ASSET_ID fish_textures[5] = {
      TEXTURE_ASSET_ID::FISH0, TEXTURE_ASSET_ID::FISH1, TEXTURE_ASSET_ID::FISH2,
      TEXTURE_ASSET_ID::FISH3, TEXTURE_ASSET_ID::FISH4};

  TEXTURE_ASSET_ID fish_texture = fish_textures[getRandInt(0, 4)];

  registry.renderRequests.insert(
      entity,
      {fish_texture, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});

  createDefaultHealthbar(renderer, entity, FISH_HEALTH, FISH_HEALTH_SCALE,
                         FISH_HEALTH_BAR_SCALE, FISH_HEALTH_BOUNDING_BOX);
  return entity;
}

/////////////////////////////////////////////////////////////////
// Sharks
/////////////////////////////////////////////////////////////////
/**
 * @brief creates a shark at a specific position sharks
 *
 * Sharks will be created with a random size
 *
 * @param renderer
 * @param position
 * @return
 */
Entity createSharkPos(RenderSystem* renderer, vec2 position) {
  // Reserve an entity
  auto entity             = Entity();
  vec2 SHARK_SCALE_FACTOR = vec2(randomFloat(SHARK_MIN_SCALE, SHARK_MAX_SCALE));

  auto& pos    = registry.positions.emplace(entity);
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = SHARK_SCALE_FACTOR * SHARK_BOUNDING_BOX;
  if (!checkSpawnCollisions(entity)) {
    // returns invalid entity, since id's start from 1
    registry.remove_all_components_of(entity);
    return Entity(0);
  }

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make enemy and damage
  registry.deadlys.emplace(entity);
  auto& damage  = registry.oxygenModifiers.emplace(entity);
  damage.amount = SHARK_DAMAGE;

  auto& modifyOxygenCd      = registry.modifyOxygenCd.emplace(entity);
  modifyOxygenCd.default_cd = SHARK_ATK_SPD;

  // Initialize the position, scale, and physics components
  auto& motion        = registry.motions.emplace(entity);
  motion.velocity     = {-SHARK_MS, 0};
  motion.acceleration = {0, 0};

  // ai
  auto& wander         = registry.wanders.emplace(entity);
  wander.active_dir_cd = 0;  // immediately picks a new direction
  wander.change_dir_cd = getRandInt(SHARK_MIN_DIR_CD, SHARK_MAX_DIR_CD);

  auto& tracking        = registry.trackPlayer.emplace(entity);
  tracking.tracking_cd  = SHARK_TRACKING_CD;
  tracking.spot_radius  = SHARK_SPOT_RADIUS;
  tracking.leash_radius = SHARK_LEASH_RADIUS;
  tracking.acceleration = SHARK_TRACKING_ACCELERATION;

  // TODO: add the room
  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::SHARK, EFFECT_ASSET_ID::TEXTURED,
               GEOMETRY_BUFFER_ID::SPRITE});

  createDefaultHealthbar(renderer, entity, SHARK_HEALTH, SHARK_HEALTH_SCALE,
                         SHARK_HEALTH_BAR_SCALE, SHARK_HEALTH_BOUNDING_BOX);
  return entity;
}

// /////////////////////////////////////////////////////////////////
// // Krabs
// /////////////////////////////////////////////////////////////////
/**
 * @brief creates a krab at a specific position
 *
 * krabs will be created with a random size
 *
 * @param renderer
 * @param position
 * @return
 */
Entity createKrabPos(RenderSystem* renderer, vec2 position) {
  // Reserve an entity
  auto entity            = Entity();
  vec2 KRAB_SCALE_FACTOR = vec2(randomFloat(KRAB_MIN_SCALE, KRAB_MAX_SCALE));

  auto& pos    = registry.positions.emplace(entity);
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = KRAB_SCALE_FACTOR * KRAB_BOUNDING_BOX;
  if (!checkSpawnCollisions(entity)) {
    // returns invalid entity, since id's start from 1
    registry.remove_all_components_of(entity);
    return Entity(0);
  }

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make enemy and damage
  registry.deadlys.emplace(entity);
  auto& damage  = registry.oxygenModifiers.emplace(entity);
  damage.amount = KRAB_DAMAGE;

  auto& modifyOxygenCd      = registry.modifyOxygenCd.emplace(entity);
  modifyOxygenCd.default_cd = KRAB_ATK_SPD;

  // Initialize the position, scale, and physics components
  auto& motion        = registry.motions.emplace(entity);
  motion.velocity     = {-KRAB_MS, 0};
  motion.acceleration = {0, 0};

  // ai
  // half of krabs will move in a line, the other half will move in a square
  if (randomSuccess(0.5)) {
    auto& wander         = registry.wanderLines.emplace(entity);
    wander.active_dir_cd = 0;  // immediately picks a new direction
    wander.change_dir_cd = getRandInt(KRAB_MIN_DIR_CD, KRAB_MAX_DIR_CD);
  } else {
    auto& wander         = registry.wanderSquares.emplace(entity);
    wander.clockwise     = randomSuccess(0.5);
    wander.active_dir_cd = 0;  // immediately picks a new direction
    wander.change_dir_cd = getRandInt(KRAB_MIN_DIR_CD, KRAB_MAX_DIR_CD);
  }

  auto& tracking        = registry.trackPlayer.emplace(entity);
  tracking.tracking_cd  = KRAB_TRACKING_CD;
  tracking.spot_radius  = KRAB_SPOT_RADIUS;
  tracking.leash_radius = KRAB_LEASH_RADIUS;
  tracking.acceleration = KRAB_TRACKING_ACCELERATION;

  // TODO: add the room
  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::KRAB, EFFECT_ASSET_ID::TEXTURED,
               GEOMETRY_BUFFER_ID::SPRITE});

  createDefaultHealthbar(renderer, entity, KRAB_HEALTH, KRAB_HEALTH_SCALE,
                         KRAB_HEALTH_BAR_SCALE, KRAB_HEALTH_BOUNDING_BOX);
  return entity;
}
