#include "enemy_factories.hpp"

#include <iostream>

#include "collision_system.hpp"
#include "components.hpp"
#include "consumable_factories.hpp"
#include "oxygen_system.hpp"
#include "physics.hpp"
#include "random.hpp"
#include "room_builder.hpp"
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
bool checkEnemySpawnCollisions(struct Position enemyPos) {
  // Entities can't spawn in the player
  for (Entity player : registry.players.entities) {
    if (!registry.positions.has(player)) {
      continue;
    }
    const Position player_pos = registry.positions.get(player);
    vec2           dist_vec   = player_pos.position - enemyPos.position;
    float          dist       = sqrt(dot(dist_vec, dist_vec));
    if (dist < PLAYER_SPAWN_RADIUS) {
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

  // Entities can't spawn in doors
  for (Entity door : registry.activeDoors.entities) {
    if (!registry.positions.has(door)) {
      continue;
    }
    const Position doorPos = registry.positions.get(door);
    if (box_collides(enemyPos, doorPos)) {
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

  // Entities can't spawn in other enemies
  for (Entity deadly : registry.deadlys.entities) {
    if (!registry.positions.has(deadly)) {
      continue;
    }
    const Position deadlypos = registry.positions.get(deadly);
    if (box_collides(enemyPos, deadlypos)) {
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
Entity createJellyPos(RenderSystem* renderer, vec2 position,
                      bool checkCollisions) {
  // Reserve an entity
  auto entity = Entity();

  Position pos;
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = JELLY_SCALE_FACTOR * JELLY_BOUNDING_BOX;

  if (checkCollisions && !checkEnemySpawnCollisions(pos)) {
    // returns invalid entity, since id's start from 1
    return Entity(0);
  }

  registry.positions.insert(entity, pos);

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make enemy
  Deadly& d   = registry.deadlys.emplace(entity);
  d.respawnFn = respawnJelly;

  // Add stats
  auto& damage  = registry.oxygenModifiers.emplace(entity);
  damage.amount = JELLY_DAMAGE;

  auto& modifyOxygenCd      = registry.modifyOxygenCd.emplace(entity);
  modifyOxygenCd.default_cd = JELLY_ATK_SPD;

  // add abilities
  auto& stun    = registry.stuns.emplace(entity);
  stun.duration = JELLY_STUN_MS;

  // assign drops
  if (randomSuccess(JELLY_DROP_CHANCE_0)) {
    Drop& drop = registry.drops.emplace(entity);
    if (randomSuccess(JELLY_DROP_CHANCE_0)) {
      drop.dropFn = createConcussiveDropPos;
    } else {
      drop.dropFn = createNetDropPos;
    }
  }

  // physics and pos

  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::JELLY, EFFECT_ASSET_ID::ENEMY,
               GEOMETRY_BUFFER_ID::SPRITE});

  createDefaultHealthbar(renderer, entity, JELLY_HEALTH, JELLY_HEALTH_SCALE,
                         JELLY_HEALTH_BAR_SCALE, JELLY_HEALTH_BOUNDING_BOX);

  return entity;
}

/**
 * @brief Respawns a Jelly based on its entity state
 *
 * @param renderer
 * @param es
 * @return
 */
Entity respawnJelly(RenderSystem* renderer, EntityState es) {
  Entity entity = createJellyPos(renderer, es.position.position, false);

  // Restore State
  Position& pos     = registry.positions.get(entity);
  pos.angle         = es.position.angle;
  pos.scale         = es.position.scale;
  pos.originalScale = es.position.originalScale;

  Oxygen& o    = registry.oxygen.get(entity);
  float   diff = es.oxygen - o.level;

  // This will also update the health bar
  if (diff < 0) {
    modifyOxygenAmount(entity, diff);
  }

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
Entity createFishPos(RenderSystem* renderer, vec2 position,
                     bool checkCollisions) {
  // Reserve an entity
  auto entity = Entity();

  Position pos;
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = FISH_SCALE_FACTOR * FISH_BOUNDING_BOX;
  if (checkCollisions && !checkEnemySpawnCollisions(pos)) {
    // returns invalid entity, since id's start from 1
    return Entity(0);
  }

  registry.positions.insert(entity, pos);

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make enemy and damage
  Deadly& d   = registry.deadlys.emplace(entity);
  d.respawnFn = respawnFish;

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

  TEXTURE_ASSET_ID fish_textures[5] = {
      TEXTURE_ASSET_ID::FISH0, TEXTURE_ASSET_ID::FISH1, TEXTURE_ASSET_ID::FISH2,
      TEXTURE_ASSET_ID::FISH3, TEXTURE_ASSET_ID::FISH4};

  TEXTURE_ASSET_ID fish_texture = fish_textures[getRandInt(0, 4)];

  registry.renderRequests.insert(entity, {fish_texture, EFFECT_ASSET_ID::ENEMY,
                                          GEOMETRY_BUFFER_ID::SPRITE});

  createDefaultHealthbar(renderer, entity, FISH_HEALTH, FISH_HEALTH_SCALE,
                         FISH_HEALTH_BAR_SCALE, FISH_HEALTH_BOUNDING_BOX);
  return entity;
}

/**
 * @brief Respawns a Fish based on its entity state
 *
 * @param renderer
 * @param es
 * @return
 */
Entity respawnFish(RenderSystem* renderer, EntityState es) {
  Entity entity = createFishPos(renderer, es.position.position, false);

  // Restore State
  Position& pos     = registry.positions.get(entity);
  pos.angle         = es.position.angle;
  pos.scale         = es.position.scale;
  pos.originalScale = es.position.originalScale;

  Oxygen& o    = registry.oxygen.get(entity);
  float   diff = es.oxygen - o.level;

  // This will also update the health bar
  if (diff < 0) {
    modifyOxygenAmount(entity, diff);
  }

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
Entity createSharkPos(RenderSystem* renderer, vec2 position,
                      bool checkCollisions) {
  // Reserve an entity
  auto entity             = Entity();
  vec2 SHARK_SCALE_FACTOR = vec2(randomFloat(SHARK_MIN_SCALE, SHARK_MAX_SCALE));

  Position pos;
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = SHARK_SCALE_FACTOR * SHARK_BOUNDING_BOX;
  if (checkCollisions && !checkEnemySpawnCollisions(pos)) {
    // returns invalid entity, since id's start from 1
    return Entity(0);
  }

  registry.positions.insert(entity, pos);

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make enemy and damage
  Deadly& d   = registry.deadlys.emplace(entity);
  d.respawnFn = respawnShark;

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

  // assign drops
  if (randomSuccess(SHARK_DROP_CHANCE_0)) {
    Drop& drop  = registry.drops.emplace(entity);
    drop.dropFn = createTorpedoDropPos;
  }

  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::SHARK, EFFECT_ASSET_ID::ENEMY,
               GEOMETRY_BUFFER_ID::SPRITE});

  createDefaultHealthbar(renderer, entity, SHARK_HEALTH, SHARK_HEALTH_SCALE,
                         SHARK_HEALTH_BAR_SCALE, SHARK_HEALTH_BOUNDING_BOX);
  return entity;
}

/**
 * @brief Respawns a Shark based on its entity state
 *
 * @param renderer
 * @param es
 * @return
 */
Entity respawnShark(RenderSystem* renderer, EntityState es) {
  Entity entity = createSharkPos(renderer, es.position.position, false);

  // Restore State
  Position& pos     = registry.positions.get(entity);
  pos.angle         = es.position.angle;
  pos.scale         = es.position.scale;
  pos.originalScale = es.position.originalScale;

  Oxygen& o    = registry.oxygen.get(entity);
  float   diff = es.oxygen - o.level;

  // This will also update the health bar
  if (diff < 0) {
    modifyOxygenAmount(entity, diff);
  }

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
Entity createKrabPos(RenderSystem* renderer, vec2 position,
                     bool checkCollisions) {
  // Reserve an entity
  auto entity            = Entity();
  vec2 KRAB_SCALE_FACTOR = vec2(randomFloat(KRAB_MIN_SCALE, KRAB_MAX_SCALE));

  Position pos;
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = KRAB_SCALE_FACTOR * KRAB_BOUNDING_BOX;
  if (checkCollisions && !checkEnemySpawnCollisions(pos)) {
    // returns invalid entity, since id's start from 1
    return Entity(0);
  }

  registry.positions.insert(entity, pos);

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make enemy and damage
  Deadly& d   = registry.deadlys.emplace(entity);
  d.respawnFn = respawnKrab;

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

  // assign drops
  if (randomSuccess(KRAB_DROP_CHANCE_0)) {
    Drop& drop  = registry.drops.emplace(entity);
    drop.dropFn = createTorpedoDropPos;
  }

  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::KRAB, EFFECT_ASSET_ID::ENEMY,
               GEOMETRY_BUFFER_ID::SPRITE});

  createDefaultHealthbar(renderer, entity, KRAB_HEALTH, KRAB_HEALTH_SCALE,
                         KRAB_HEALTH_BAR_SCALE, KRAB_HEALTH_BOUNDING_BOX);
  return entity;
}

/**
 * @brief Respawns a Krab based on its entity state
 *
 * @param renderer
 * @param es
 * @return
 */
Entity respawnKrab(RenderSystem* renderer, EntityState es) {
  Entity entity = createKrabPos(renderer, es.position.position, false);

  // Restore State
  Position& pos     = registry.positions.get(entity);
  pos.angle         = es.position.angle;
  pos.scale         = es.position.scale;
  pos.originalScale = es.position.originalScale;

  Oxygen& o    = registry.oxygen.get(entity);
  float   diff = es.oxygen - o.level;

  // This will also update the health bar
  if (diff < 0) {
    modifyOxygenAmount(entity, diff);
  }

  return entity;
}

// /////////////////////////////////////////////////////////////////
// // Urchins
// /////////////////////////////////////////////////////////////////
/**
 * @brief creates an urchin at a specific position
 *
 * urchins will be created with a random size
 *
 * @param renderer
 * @param position
 * @return
 */
Entity createUrchinPos(RenderSystem* renderer, vec2 position,
                       bool checkCollisions) {
  // Reserve an entity
  auto entity = Entity();
  vec2 URCHIN_SCALE_FACTOR =
      vec2(randomFloat(URCHIN_MIN_SCALE, URCHIN_MAX_SCALE));

  Position pos;
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = URCHIN_SCALE_FACTOR * URCHIN_BOUNDING_BOX;
  if (checkCollisions && !checkEnemySpawnCollisions(pos)) {
    // returns invalid entity, since id's start from 1
    return Entity(0);
  }
  registry.positions.insert(entity, pos);

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make enemy and damage
  Deadly& d   = registry.deadlys.emplace(entity);
  d.respawnFn = respawnUrchin;

  // Initialize the position, scale, and physics components
  auto& motion        = registry.motions.emplace(entity);
  motion.velocity     = {-URCHIN_MS, 0};
  motion.acceleration = {0, 0};

  // ai
  auto& wander         = registry.wanders.emplace(entity);
  wander.active_dir_cd = 0;  // immediately picks a new direction
  wander.change_dir_cd = getRandInt(URCHIN_MIN_DIR_CD, URCHIN_MAX_DIR_CD);

  auto& shooter      = registry.shooters.emplace(entity);
  shooter.type       = RangedEnemies::URCHIN;
  shooter.default_cd = URCHIN_FIRERATE;
  shooter.cooldown   = URCHIN_FIRERATE;

  // assign drops
  if (randomSuccess(URCHIN_DROP_CHANCE_0)) {
    Drop& drop  = registry.drops.emplace(entity);
    drop.dropFn = createTorpedoDropPos;
  }

  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::URCHIN, EFFECT_ASSET_ID::ENEMY,
               GEOMETRY_BUFFER_ID::SPRITE});

  createDefaultHealthbar(renderer, entity, URCHIN_HEALTH, URCHIN_HEALTH_SCALE,
                         URCHIN_HEALTH_BAR_SCALE, URCHIN_HEALTH_BOUNDING_BOX);
  return entity;
}

/**
 * @brief Respawns an urchin based on its entity state
 *
 * @param renderer
 * @param es
 * @return
 */
Entity respawnUrchin(RenderSystem* renderer, EntityState es) {
  Entity entity = createUrchinPos(renderer, es.position.position, false);

  // Restore State
  Position& pos     = registry.positions.get(entity);
  pos.angle         = es.position.angle;
  pos.scale         = es.position.scale;
  pos.originalScale = es.position.originalScale;

  Oxygen& o    = registry.oxygen.get(entity);
  float   diff = es.oxygen - o.level;

  // This will also update the health bar
  if (diff < 0) {
    modifyOxygenAmount(entity, diff);
  }

  return entity;
}

Entity launchUrchinNeedle(RenderSystem* renderer, vec2 pos, float angle) {
  auto entity = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // Setting initial position values
  Position& position = registry.positions.emplace(entity);
  position.position  = pos;
  position.scale     = URCHIN_NEEDLE_SCALE_FACTOR * URCHIN_NEEDLE_BOUNDING_BOX;
  position.angle     = angle;

  // Setting initial motion values
  Motion& motion  = registry.motions.emplace(entity);
  motion.velocity = {cos(angle) * URCHIN_NEEDLE_MS,
                     sin(angle) * URCHIN_NEEDLE_MS};

  OxygenModifier& oxyCost = registry.oxygenModifiers.emplace(entity);
  oxyCost.amount          = URCHIN_NEEDLE_DAMAGE;

  EnemyProjectile& proj = registry.enemyProjectiles.emplace(entity);
  proj.has_timer        = true;
  proj.timer            = URCHIN_NEEDLE_TIMER;

  // Request Render
  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::URCHIN_NEEDLE, EFFECT_ASSET_ID::ENEMY,
               GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}

// /////////////////////////////////////////////////////////////////
// // Seahorse
// /////////////////////////////////////////////////////////////////
/**
 * @brief creates a seahorse at a specific position
 *
 * seahorses will be created with a random size
 *
 * @param renderer
 * @param position
 * @return
 */
Entity createSeahorsePos(RenderSystem* renderer, vec2 position,
                         bool checkCollisions) {
  // Reserve an entity
  auto entity = Entity();
  vec2 SEAHORSE_SCALE_FACTOR =
      vec2(randomFloat(SEAHORSE_MIN_SCALE, SEAHORSE_MAX_SCALE));

  Position pos;
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = SEAHORSE_SCALE_FACTOR * SEAHORSE_BOUNDING_BOX;
  if (checkCollisions && !checkEnemySpawnCollisions(pos)) {
    // returns invalid entity, since id's start from 1
    return Entity(0);
  }
  registry.positions.insert(entity, pos);

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make enemy and damage
  Deadly& d   = registry.deadlys.emplace(entity);
  d.respawnFn = respawnSeahorse;

  // Initialize the position, scale, and physics components
  auto& motion        = registry.motions.emplace(entity);
  motion.velocity     = {-SEAHORSE_MS, 0};
  motion.acceleration = {0, 0};

  // ai
  auto& wander         = registry.wanders.emplace(entity);
  wander.active_dir_cd = 0;  // immediately picks a new direction
  wander.change_dir_cd = getRandInt(SEAHORSE_MIN_DIR_CD, SEAHORSE_MAX_DIR_CD);

  auto& shooter      = registry.shooters.emplace(entity);
  shooter.type       = RangedEnemies::SEAHORSE;
  shooter.default_cd = SEAHORSE_FIRERATE;
  shooter.cooldown   = SEAHORSE_FIRERATE;

  // assign drops
  if (randomSuccess(SEAHORSE_DROP_CHANCE_0)) {
    Drop& drop  = registry.drops.emplace(entity);
    drop.dropFn = createShrimpDropPos;
  }

  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::SEAHORSE, EFFECT_ASSET_ID::ENEMY,
               GEOMETRY_BUFFER_ID::SPRITE});

  createDefaultHealthbar(renderer, entity, SEAHORSE_HEALTH,
                         SEAHORSE_HEALTH_SCALE, SEAHORSE_HEALTH_BAR_SCALE,
                         SEAHORSE_HEALTH_BOUNDING_BOX);
  return entity;
}

/**
 * @brief Respawns a Seahorse based on its entity state
 *
 * @param renderer
 * @param es
 * @return
 */
Entity respawnSeahorse(RenderSystem* renderer, EntityState es) {
  Entity entity = createSeahorsePos(renderer, es.position.position, false);

  // Restore State
  Position& pos     = registry.positions.get(entity);
  pos.angle         = es.position.angle;
  pos.scale         = es.position.scale;
  pos.originalScale = es.position.originalScale;

  Oxygen& o    = registry.oxygen.get(entity);
  float   diff = es.oxygen - o.level;

  // This will also update the health bar
  if (diff < 0) {
    modifyOxygenAmount(entity, diff);
  }

  return entity;
}

Entity fireSeahorseBullet(RenderSystem* renderer, vec2 pos, vec2 direction) {
  auto entity = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // Setting initial position values
  float     angle    = atan2(direction.y, direction.x);
  Position& position = registry.positions.emplace(entity);
  position.position  = pos;
  position.scale = SEAHORSE_BULLET_SCALE_FACTOR * SEAHORSE_BULLET_BOUNDING_BOX;
  position.angle = angle;

  // Setting initial motion values
  Motion& motion  = registry.motions.emplace(entity);
  motion.velocity = {cos(angle) * SEAHORSE_BULLET_MS,
                     sin(angle) * SEAHORSE_BULLET_MS};

  OxygenModifier& oxyCost = registry.oxygenModifiers.emplace(entity);
  oxyCost.amount          = SEAHORSE_BULLET_DAMAGE;

  EnemyProjectile& proj = registry.enemyProjectiles.emplace(entity);
  proj.has_timer        = true;
  proj.timer            = SEAHORSE_BULLET_TIMER;

  // Request Render
  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::SEAHORSE_BULLET, EFFECT_ASSET_ID::ENEMY,
               GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}

// /////////////////////////////////////////////////////////////////
// // Lobster
// /////////////////////////////////////////////////////////////////
/**
 * @brief creates a lobster at a specific position
 *
 *
 * @param renderer
 * @param position
 * @return
 */
Entity createLobsterPos(RenderSystem* renderer, vec2 position,
                        bool checkCollisions) {
  // Reserve an entity
  auto entity               = Entity();
  vec2 LOBSTER_SCALE_FACTOR = vec2(LOBSTER_SCALE);

  auto& pos    = registry.positions.emplace(entity);
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = LOBSTER_SCALE_FACTOR * LOBSTER_BOUNDING_BOX;
  if (checkCollisions && !checkEnemySpawnCollisions(pos)) {
    // returns invalid entity, since id's start from 1
    registry.remove_all_components_of(entity);
    return Entity(0);
  }

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make enemy and damage
  Deadly& d   = registry.deadlys.emplace(entity);
  d.respawnFn = respawnLobster;

  auto& damage  = registry.oxygenModifiers.emplace(entity);
  damage.amount = LOBSTER_DAMAGE;

  auto& modifyOxygenCd      = registry.modifyOxygenCd.emplace(entity);
  modifyOxygenCd.default_cd = LOBSTER_ATK_SPD;

  // Initialize the position, scale, and physics components
  auto& motion        = registry.motions.emplace(entity);
  motion.velocity     = {-LOBSTER_MS, 0};
  motion.acceleration = {0, 0};

  // ai
  auto& wander         = registry.wanderSquares.emplace(entity);
  wander.clockwise     = randomSuccess(0.5);
  wander.active_dir_cd = 0;  // immediately picks a new direction
  wander.change_dir_cd = getRandInt(LOBSTER_MIN_DIR_CD, LOBSTER_MAX_DIR_CD);

  auto& tracking        = registry.trackPlayer.emplace(entity);
  tracking.tracking_cd  = LOBSTER_TRACKING_CD;
  tracking.spot_radius  = LOBSTER_SPOT_RADIUS;
  tracking.leash_radius = LOBSTER_LEASH_RADIUS;
  tracking.acceleration = LOBSTER_TRACKING_ACCELERATION;

  Lobster& lobster_comp         = registry.lobsters.emplace(entity);
  lobster_comp.original_speed   = LOBSTER_MS;
  lobster_comp.block_mitigation = LOBSTER_BLOCK_MITIGATION;
  lobster_comp.block_duration   = LOBSTER_BLOCK_DURATION;
  lobster_comp.ram_duration     = LOBSTER_RAM_DURATION;
  lobster_comp.ram_speed        = LOBSTER_RAM_SPEED;

  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::LOBSTER, EFFECT_ASSET_ID::ENEMY,
               GEOMETRY_BUFFER_ID::SPRITE});

  createDefaultHealthbar(renderer, entity, LOBSTER_HEALTH, LOBSTER_HEALTH_SCALE,
                         LOBSTER_HEALTH_BAR_SCALE, LOBSTER_HEALTH_BOUNDING_BOX);
  return entity;
}

/**
 * @brief Respawns a Lobster based on its entity state
 *
 * @param renderer
 * @param es
 * @return
 */
Entity respawnLobster(RenderSystem* renderer, EntityState es) {
  Entity entity = createLobsterPos(renderer, es.position.position, false);

  // Restore State
  Position& pos     = registry.positions.get(entity);
  pos.angle         = es.position.angle;
  pos.scale         = es.position.scale;
  pos.originalScale = es.position.originalScale;

  Oxygen& o    = registry.oxygen.get(entity);
  float   diff = es.oxygen - o.level;

  // This will also update the health bar
  if (diff < 0) {
    modifyOxygenAmount(entity, diff);
  }

  return entity;
}