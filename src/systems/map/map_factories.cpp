#include "map_factories.hpp"
#include "physics_system.hpp"

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
  const Position &entityPos = registry.positions.get(entity);

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
Entity createGeyserPos(RenderSystem *renderer, vec2 position) {
  // Reserve an entity
  auto entity = Entity();

  auto &pos = registry.positions.emplace(entity);
  pos.angle = 0.f;
  pos.position = position;
  pos.scale = GEYSER_SCALE_FACTOR * GEYSER_BOUNDING_BOX;

  if (!checkSpawnCollisions(entity)) {
    // returns invalid entity, since id's start from 1
    registry.remove_all_components_of(entity);
    return Entity(0);
  }

  // Store a reference to the potentially re-used mesh object
  Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make consumable
  registry.interactable.emplace(entity);
  registry.collidables.emplace(entity);

  // Add stats
  auto &damage = registry.damageTouch.emplace(entity);
  damage.amount = GEYSER_QTY;
  auto &attackCD = registry.attackCD.emplace(entity);
  attackCD.attack_spd = GEYSER_RATE_MS;

  // physics and pos

  registry.renderRequests.insert(entity, {TEXTURE_ASSET_ID::GEYSER,
                                          EFFECT_ASSET_ID::TEXTURED,
                                          GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}
  // physics and pos
  auto &pos = registry.positions.emplace(entity);
  pos.angle = 0.f;
  pos.position = position;
  pos.scale = GEYSER_SCALE_FACTOR * GEYSER_BOUNDING_BOX;

  registry.renderRequests.insert(entity, {TEXTURE_ASSET_ID::GEYSER,
                                          EFFECT_ASSET_ID::TEXTURED,
                                          GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}

