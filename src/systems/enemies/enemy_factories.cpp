#include "enemy_factories.hpp"
#include "tiny_ecs_registry.hpp"
/////////////////////////////////////////////////////////////////
// Jellyfish
/////////////////////////////////////////////////////////////////
/**
 * @brief creates a jellyfish in a room
  // TODO: add the room as arg as well
 *
 * @param renderer
 * @param randPos - function that returns a random position in a room
 * @return 
 */
Entity createJellyRoom(RenderSystem *renderer, vec2 (*randPos)(void)) {
  vec2 pos = randPos();
  return createFishPos(renderer, pos);
}

/**
 * @brief creates a fish at a specific position
 *
 * @param renderer 
 * @param position 
 * @return 
 */
Entity createJellyPos(RenderSystem *renderer, vec2 position) {
  // Reserve en entity
  auto entity = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make enemy
  auto &deadly = registry.deadlys.emplace(entity);

  // Add stats
  auto &damage = registry.damageTouch.emplace(entity);
  damage.damage = JELLY_DAMAGE;
  auto &oxygen = registry.oxygen.emplace(entity);
  oxygen.level = JELLY_OXYGEN;

  // add abilities
  auto &stun = registry.stuns.emplace(entity);
  stun.duration = JELLY_STUN_MS;

  // physics and pos
  auto &pos = registry.positions.emplace(entity);
  pos.angle = 0.f;
  pos.position = position;
  pos.scale = {-JELLY_BB_WIDTH, JELLY_BB_HEIGHT};



  registry.renderRequests.insert(entity, {TEXTURE_ASSET_ID::PLAYER, // Placeholder ID
                                          EFFECT_ASSET_ID::TEXTURED,
                                          GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}
/////////////////////////////////////////////////////////////////
// Fish
/////////////////////////////////////////////////////////////////
/**
 * @brief creates a fish in a room
  // TODO: add the room as arg as well
 *
 * @param renderer
 * @param randPos - function that returns a random position in a room
 * @return 
 */
Entity createFishRoom(RenderSystem *renderer, vec2 (*randPos)(void)) {
  vec2 pos = randPos();
  return createFishPos(renderer, pos);
}

/**
 * @brief creates a fish at a specific position
 *
 * @param renderer 
 * @param position 
 * @return 
 */
Entity createFishPos(RenderSystem *renderer, vec2 position) {
  // Reserve en entity
  auto entity = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make enemy and damage
  auto &deadly = registry.deadlys.emplace(entity);
  auto &damage = registry.damageTouch.emplace(entity);
  damage.damage = FISH_DAMAGE;

  auto &oxygen = registry.oxygen.emplace(entity);
  oxygen.level = FISH_OXYGEN;

  // Initialize the position, scale, and physics components
  auto &motion = registry.motions.emplace(entity);
  motion.velocity = {-FISH_MS, 0};
  motion.acceleration = {0, 0};

  auto &pos = registry.positions.emplace(entity);
  pos.angle = 0.f;
  pos.position = position;
  pos.scale = {-FISH_BB_WIDTH, FISH_BB_HEIGHT};

  // ai
  auto &wander = registry.wanders.emplace(entity);

  // TODO: add the room

  registry.renderRequests.insert(entity, {TEXTURE_ASSET_ID::PLAYER, // Placeholder ID
                                          EFFECT_ASSET_ID::TEXTURED,
                                          GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}
