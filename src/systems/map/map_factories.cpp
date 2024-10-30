#include "map_factories.hpp"

#include <iostream>

#include "collision_system.hpp"
#include "enemy_factories.hpp"
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

  createCrateHealthBar(renderer, entity);

  // assign drops
  if (randomSuccess(CRATE_DROP_CHANCE_0)) {
    Drop &drop = registry.drops.emplace(entity);
    drop.dropFn = CRATE_DROP_0;
    return entity;
  }

  return entity;
}

/**
 * @brief creates a health bar for a crate
 *
 * @param renderer
 * @param crate - assumed to be a crate
 * @return
 */
void createCrateHealthBar(RenderSystem* renderer, Entity& crate) {
  // Check if crate has a position component
  if (!registry.positions.has(crate)) {
    std::cerr << "Error: Entity does not have a position component"
              << std::endl;
    return;
  }

  // Create oxygen and background bar
  auto crateOxygenBar     = Entity();
  auto crateBackgroundBar = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(crateOxygenBar, &mesh);
  registry.meshPtrs.emplace(crateBackgroundBar, &mesh);

  // Get position of crate
  Position& cratePos = registry.positions.get(crate);

  // Setting initial positon values
  Position& position = registry.positions.emplace(crateOxygenBar);
  position.position =
      cratePos.position -
      vec2(0.f, cratePos.scale.y / 2 +
                    ENEMY_O2_BAR_GAP);  // TODO: guesstimate on where the HP
                                        // should be, update to proper position
  position.angle         = 0.f;
  position.scale         = CRATE_HEALTH_SCALE * CRATE_HEALTH_BOUNDING_BOX;
  position.originalScale = CRATE_HEALTH_SCALE * CRATE_HEALTH_BOUNDING_BOX;

  Position& backgroundPos = registry.positions.emplace(crateBackgroundBar);
  backgroundPos.position  = position.position;
  backgroundPos.angle     = 0.f;
  backgroundPos.scale     = CRATE_HEALTH_BAR_SCALE * CRATE_HEALTH_BOUNDING_BOX;

  // Set health bar
  auto& crateOxygen         = registry.oxygen.emplace(crate);
  crateOxygen.capacity      = CRATE_HEALTH;
  crateOxygen.level         = CRATE_HEALTH;
  crateOxygen.rate          = 0.f;
  crateOxygen.oxygenBar     = crateOxygenBar;
  crateOxygen.backgroundBar = crateBackgroundBar;

  // TODO: change to proper texture
  registry.renderRequests.insert(
      crateOxygenBar, {TEXTURE_ASSET_ID::ENEMY_OXYGEN_BAR,
                       EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});

  registry.renderRequests.insert(
      crateBackgroundBar,
      {TEXTURE_ASSET_ID::ENEMY_BACKGROUND_BAR, EFFECT_ASSET_ID::TEXTURED,
       GEOMETRY_BUFFER_ID::SPRITE});
}
