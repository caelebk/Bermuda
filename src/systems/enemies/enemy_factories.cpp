#include "enemy_factories.hpp"
#include "physics.hpp"
#include "collision_system.hpp"
#include "space.hpp"
#include "tiny_ecs_registry.hpp"

#include <iostream>

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
  const Position &enemyPos = registry.positions.get(entity);

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
Entity createJellyPos(RenderSystem *renderer, vec2 position) {
  // Reserve an entity
  auto entity = Entity();

  auto &pos = registry.positions.emplace(entity);
  pos.angle = 0.f;
  pos.position = position;
  pos.scale = JELLY_SCALE_FACTOR * JELLY_BOUNDING_BOX;

  if (!checkSpawnCollisions(entity)) {
    // returns invalid entity, since id's start from 1
    registry.remove_all_components_of(entity);
    return Entity(0);
  }

  // Store a reference to the potentially re-used mesh object
  Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make enemy
  registry.deadlys.emplace(entity);

  // Add stats
  auto &damage = registry.damageTouch.emplace(entity);
  damage.amount = JELLY_DAMAGE;

  auto &attackCD = registry.attackCD.emplace(entity);
  attackCD.attack_spd = JELLY_ATK_SPD;

  // add abilities
  auto &stun = registry.stuns.emplace(entity);
  stun.duration = JELLY_STUN_MS;

  // physics and pos

  // add collisions
  registry.collidables.emplace(entity);

  registry.renderRequests.insert(entity, {TEXTURE_ASSET_ID::JELLY,
                                          EFFECT_ASSET_ID::TEXTURED,
                                          GEOMETRY_BUFFER_ID::SPRITE});

  createJellyHealthBar(renderer, entity);

  return entity;
}

/**
 * @brief creates a health bar for a jellyfish
 *
 * @param renderer
 * @param enemy - assumed to be a jellyfish
 * @return
 */
void createJellyHealthBar(RenderSystem *renderer, Entity &enemy) {
  // Check if enemy has a position component
  if (!registry.positions.has(enemy)) {
    std::cerr << "Error: Entity does not have a position component"
              << std::endl;
    return;
  }

  // Create oxygen and background bar
  auto jellyOxygenBar = Entity();
  auto jellyBackgroundBar = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(jellyOxygenBar, &mesh);
  registry.meshPtrs.emplace(jellyBackgroundBar, &mesh);

  // Get position of enemy
  Position &enemyPos = registry.positions.get(enemy);

  // Setting initial positon values
  Position &position = registry.positions.emplace(jellyOxygenBar);
  position.position =
      enemyPos.position -
      vec2(0.f, enemyPos.scale.y / 2 +
                    ENEMY_O2_BAR_GAP); // TODO: guesstimate on where the HP
                                       // should be, update to proper position
  position.angle = 0.f;
  position.scale = JELLY_HEALTH_SCALE * JELLY_HEALTH_BOUNDING_BOX;
  position.originalScale = JELLY_HEALTH_SCALE * JELLY_HEALTH_BOUNDING_BOX;

  Position &backgroundPos = registry.positions.emplace(jellyBackgroundBar);
  backgroundPos.position = position.position;
  backgroundPos.angle = 0.f;
  backgroundPos.scale = JELLY_HEALTH_BAR_SCALE * JELLY_HEALTH_BOUNDING_BOX;

  // Set health bar
  auto &jellyOxygen = registry.oxygen.emplace(enemy);
  jellyOxygen.capacity = JELLY_HEALTH;
  jellyOxygen.level = JELLY_HEALTH;
  jellyOxygen.rate = 0.f;
  jellyOxygen.oxygenBar = jellyOxygenBar;
  jellyOxygen.backgroundBar = jellyBackgroundBar;

  // TODO: change to proper texture
  registry.renderRequests.insert(
      jellyOxygenBar, {TEXTURE_ASSET_ID::ENEMY_OXYGEN_BAR,
                       EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});

  registry.renderRequests.insert(jellyBackgroundBar,
                                 {TEXTURE_ASSET_ID::ENEMY_BACKGROUND_BAR,
                                  EFFECT_ASSET_ID::TEXTURED,
                                  GEOMETRY_BUFFER_ID::SPRITE});
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
Entity createFishPos(RenderSystem *renderer, vec2 position) {
  // Reserve an entity
  auto entity = Entity();

  auto &pos = registry.positions.emplace(entity);
  pos.angle = 0.f;
  pos.position = position;
  pos.scale = FISH_SCALE_FACTOR * FISH_BOUNDING_BOX;
  if (!checkSpawnCollisions(entity)) {
    // returns invalid entity, since id's start from 1
    registry.remove_all_components_of(entity);
    return Entity(0);
  }

  // Store a reference to the potentially re-used mesh object
  Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make enemy and damage
  registry.deadlys.emplace(entity);
  auto &damage = registry.damageTouch.emplace(entity);
  damage.amount = FISH_DAMAGE;

  auto &attackCD = registry.attackCD.emplace(entity);
  attackCD.attack_spd = FISH_ATK_SPD;

  // Initialize the position, scale, and physics components
  auto &motion = registry.motions.emplace(entity);
  motion.velocity = {-FISH_MS, 0};
  motion.acceleration = {0, 0};

  // add collisions
  registry.collidables.emplace(entity);

  // ai
  // auto &wander = registry.wanders.emplace(entity);

  // TODO: add the room

  registry.renderRequests.insert(entity, {TEXTURE_ASSET_ID::FISH,
                                          EFFECT_ASSET_ID::TEXTURED,
                                          GEOMETRY_BUFFER_ID::SPRITE});

  createFishHealthBar(renderer, entity);
  return entity;
}

/**
 * @brief creates a health bar for a fish
 *
 * @param renderer
 * @param enemy - assumed to be a fish
 * @return
 */
void createFishHealthBar(RenderSystem *renderer, Entity &enemy) {
  // Check if enemy has a position component
  if (!registry.positions.has(enemy)) {
    std::cerr << "Error: Entity does not have a position component"
              << std::endl;
    return;
  }

  // Create oxygen and background bar
  auto fishOxygenBar = Entity();
  auto fishBackgroundBar = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(fishOxygenBar, &mesh);
  registry.meshPtrs.emplace(fishBackgroundBar, &mesh);

  // Get position of enemy
  Position &enemyPos = registry.positions.get(enemy);

  // Setting initial positon values
  Position &position = registry.positions.emplace(fishOxygenBar);
  position.position =
      enemyPos.position -
      vec2(0.f, enemyPos.scale.y / 2 +
                    ENEMY_O2_BAR_GAP); // TODO: guesstimate on where the HP
                                       // should be, update to proper position
  position.angle = 0.f;
  position.scale = FISH_HEALTH_SCALE * FISH_HEALTH_BOUNDING_BOX;
  position.originalScale = FISH_HEALTH_SCALE * FISH_HEALTH_BOUNDING_BOX;

  Position &backgroundPos = registry.positions.emplace(fishBackgroundBar);
  backgroundPos.position = position.position;
  backgroundPos.angle = 0.f;
  backgroundPos.scale = FISH_HEALTH_BAR_SCALE * FISH_HEALTH_BOUNDING_BOX;

  // Set health bar
  auto &oxygen = registry.oxygen.emplace(enemy);
  oxygen.capacity = FISH_HEALTH;
  oxygen.level = FISH_HEALTH;
  oxygen.rate = 0.f;
  oxygen.oxygenBar = fishOxygenBar;
  oxygen.backgroundBar = fishBackgroundBar;

  // TODO: change to proper texture
  registry.renderRequests.insert(
      fishOxygenBar, {TEXTURE_ASSET_ID::ENEMY_OXYGEN_BAR,
                      EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});

  registry.renderRequests.insert(fishBackgroundBar,
                                 {TEXTURE_ASSET_ID::ENEMY_BACKGROUND_BAR,
                                  EFFECT_ASSET_ID::TEXTURED,
                                  GEOMETRY_BUFFER_ID::SPRITE});
}

/////////////////////////////////////////////////////////////////
// Sharks
/////////////////////////////////////////////////////////////////

// /**
//  * @brief creates a health bar for a shark
//  *
//  * @param renderer
//  * @param enemy - assumed to be a shark
//  * @return entity id if successful, -1 otherwise
//  */
// int createSharkHealthBar(RenderSystem *renderer, Entity enemy)
// {
//     // Check if enemy has a position component
//     if (!registry.positions.has(enemy))
//     {
//         std::cerr << "Error: Entity does not have a position component" <<
//         std::endl; return -1;
//     }

//     auto entity = Entity();

//     // Store a reference to the potentially re-used mesh object
//     Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
//     registry.meshPtrs.emplace(entity, &mesh);

//     // Get position of enemy
//     Position &enemyPos = registry.positions.get(enemy);

//     // Setting initial positon values
//     Position &position = registry.positions.emplace(entity);
//     position.position = enemyPos.position - vec2(0.f, enemyPos.scale.y / 2 +
//     ENEMY_O2_BAR_GAP); // TODO: guesstimate on where the HP should be, update
//     to proper position position.angle = 0.f; position.scale =
//     SHARK_OXYGEN_SCALE;

//     // Set health bar
//     auto &health = registry.oxygen.emplace(entity);
//     oxygen.capacity = SHARK_OXYGEN;
//     oxygen.level = SHARK_OXYGEN;
//     oxygen.rate = 0.f;

//     // TODO: change to proper texture
//     registry.renderRequests.insert(
//         entity,
//         {TEXTURE_ASSET_ID::TEXTURE_COUNT,
//          EFFECT_ASSET_ID::TEXTURED,
//          GEOMETRY_BUFFER_ID::SPRITE});

//     return entity.operator unsigned int();
// }

// /////////////////////////////////////////////////////////////////
// // Octopi
// /////////////////////////////////////////////////////////////////

// /**
//  * @brief creates a health bar for an octopus
//  *
//  * @param renderer
//  * @param enemy - assumed to be an octopus
//  * @return entity id if successful, -1 otherwise
//  */
// int createOctopusHealthBar(RenderSystem *renderer, Entity enemy)
// {
//     // Check if enemy has a position component
//     if (!registry.positions.has(enemy))
//     {
//         std::cerr << "Error: Entity does not have a position component" <<
//         std::endl; return -1;
//     }

//     auto entity = Entity();

//     // Store a reference to the potentially re-used mesh object
//     Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
//     registry.meshPtrs.emplace(entity, &mesh);

//     // Get position of enemy
//     Position &enemyPos = registry.positions.get(enemy);

//     // Setting initial positon values
//     Position &position = registry.positions.emplace(entity);
//     position.position = enemyPos.position - vec2(0.f, enemyPos.scale.y / 2 +
//     ENEMY_O2_BAR_GAP); // TODO: guesstimate on where the HP should be, update
//     to proper position position.angle = 0.f; position.scale =
//     OCTOPUS_OXYGEN_SCALE;

//     // Set health bar
//     auto &health = registry.oxygen.emplace(entity);
//     oxygen.capacity = OCTOPUS_OXYGEN;
//     oxygen.level = OCTOPUS_OXYGEN;
//     oxygen.rate = 0.f;

//     // TODO: change to proper texture
//     registry.renderRequests.insert(
//         entity,
//         {TEXTURE_ASSET_ID::TEXTURE_COUNT,
//          EFFECT_ASSET_ID::TEXTURED,
//          GEOMETRY_BUFFER_ID::SPRITE});

//     return entity.operator unsigned int();
// }

// /////////////////////////////////////////////////////////////////
// // Krabs
// /////////////////////////////////////////////////////////////////

// /**
//  * @brief creates a health bar for a krab
//  *
//  * @param renderer
//  * @param enemy - assumed to be a krab
//  * @return entity id if successful, -1 otherwise
//  */
// int createKrabHealthBar(RenderSystem *renderer, Entity enemy)
// {
//     // Check if enemy has a position component
//     if (!registry.positions.has(enemy))
//     {
//         std::cerr << "Error: Entity does not have a position component" <<
//         std::endl; return -1;
//     }

//     auto entity = Entity();

//     // Store a reference to the potentially re-used mesh object
//     Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
//     registry.meshPtrs.emplace(entity, &mesh);

//     // Get position of enemy
//     Position &enemyPos = registry.positions.get(enemy);

//     // Setting initial positon values
//     Position &position = registry.positions.emplace(entity);
//     position.position = enemyPos.position - vec2(0.f, enemyPos.scale.y / 2 +
//     ENEMY_O2_BAR_GAP); // TODO: guesstimate on where the HP should be, update
//     to proper position position.angle = 0.f; position.scale =
//     KRAB_OXYGEN_SCALE;

//     // Set health bar
//     auto &health = registry.oxygen.emplace(entity);
//     oxygen.capacity = KRAB_OXYGEN;
//     oxygen.level = KRAB_OXYGEN;
//     oxygen.rate = 0.f;

//     // TODO: change to proper texture
//     registry.renderRequests.insert(
//         entity,
//         {TEXTURE_ASSET_ID::TEXTURE_COUNT,
//          EFFECT_ASSET_ID::TEXTURED,
//          GEOMETRY_BUFFER_ID::SPRITE});

//     return entity.operator unsigned int();
// }

// /////////////////////////////////////////////////////////////////
// // Sea mine
// /////////////////////////////////////////////////////////////////

// /**
//  * @brief creates a health bar for a sea mine
//  *
//  * @param renderer
//  * @param enemy - assumed to be a sea mine
//  * @return entity id if successful, -1 otherwise
//  */
// int createSeaMineHealthBar(RenderSystem *renderer, Entity enemy)
// {
//     // Check if enemy has a position component
//     if (!registry.positions.has(enemy))
//     {
//         std::cerr << "Error: Entity does not have a position component" <<
//         std::endl; return -1;
//     }

//     auto entity = Entity();

//     // Store a reference to the potentially re-used mesh object
//     Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
//     registry.meshPtrs.emplace(entity, &mesh);

//     // Get position of enemy
//     Position &enemyPos = registry.positions.get(enemy);

//     // Setting initial positon values
//     Position &position = registry.positions.emplace(entity);
//     position.position = enemyPos.position - vec2(0.f, enemyPos.scale.y / 2 +
//     ENEMY_O2_BAR_GAP); // TODO: guesstimate on where the HP should be, update
//     to proper position position.angle = 0.f; position.scale =
//     SEA_MINE_OXYGEN_SCALE;

//     // Set health bar
//     auto &health = registry.oxygen.emplace(entity);
//     oxygen.capacity = SEA_MINE_OXYGEN;
//     oxygen.level = SEA_MINE_OXYGEN;
//     oxygen.rate = 0.f;

//     // TODO: change to proper texture
//     registry.renderRequests.insert(
//         entity,
//         {TEXTURE_ASSET_ID::TEXTURE_COUNT,
//          EFFECT_ASSET_ID::TEXTURED,
//          GEOMETRY_BUFFER_ID::SPRITE});

//     return entity.operator unsigned int();
// }

// /////////////////////////////////////////////////////////////////
// // Merpeople
// /////////////////////////////////////////////////////////////////

// /**
//  * @brief creates a health bar for a merperson
//  *
//  * @param renderer
//  * @param enemy - assumed to be a merperson
//  * @return entity id if successful, -1 otherwise
//  */
// int createMerpersonHealthBar(RenderSystem *renderer, Entity enemy)
// {
//     // Check if enemy has a position component
//     if (!registry.positions.has(enemy))
//     {
//         std::cerr << "Error: Entity does not have a position component" <<
//         std::endl; return -1;
//     }

//     auto entity = Entity();

//     // Store a reference to the potentially re-used mesh object
//     Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
//     registry.meshPtrs.emplace(entity, &mesh);

//     // Get position of enemy
//     Position &enemyPos = registry.positions.get(enemy);

//     // Setting initial positon values
//     Position &position = registry.positions.emplace(entity);
//     position.position = enemyPos.position - vec2(0.f, enemyPos.scale.y / 2 +
//     ENEMY_O2_BAR_GAP); // TODO: guesstimate on where the HP should be, update
//     to proper position position.angle = 0.f; position.scale =
//     MERPERSON_OXYGEN_SCALE;

//     // Set health bar
//     auto &health = registry.oxygen.emplace(entity);
//     oxygen.capacity = MERPERSON_OXYGEN;
//     oxygen.level = MERPERSON_OXYGEN;
//     oxygen.rate = 0.f;

//     // TODO: change to proper texture
//     registry.renderRequests.insert(
//         entity,
//         {TEXTURE_ASSET_ID::TEXTURE_COUNT,
//          EFFECT_ASSET_ID::TEXTURED,
//          GEOMETRY_BUFFER_ID::SPRITE});

//     return entity.operator unsigned int();
// }

// /////////////////////////////////////////////////////////////////
// // Void Tentacles
// /////////////////////////////////////////////////////////////////

// /**
//  * @brief creates a health bar for a tentacle
//  *
//  * @param renderer
//  * @param enemy - assumed to be a tentacle
//  * @return entity id if successful, -1 otherwise
//  */
// int createTentacleHealthBar(RenderSystem *renderer, Entity enemy)
// {
//     // Check if enemy has a position component
//     if (!registry.positions.has(enemy))
//     {
//         std::cerr << "Error: Entity does not have a position component" <<
//         std::endl; return -1;
//     }

//     auto entity = Entity();

//     // Store a reference to the potentially re-used mesh object
//     Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
//     registry.meshPtrs.emplace(entity, &mesh);

//     // Get position of enemy
//     Position &enemyPos = registry.positions.get(enemy);

//     // Setting initial positon values
//     Position &position = registry.positions.emplace(entity);
//     position.position = enemyPos.position - vec2(0.f, enemyPos.scale.y / 2 +
//     ENEMY_O2_BAR_GAP); // TODO: guesstimate on where the HP should be, update
//     to proper position position.angle = 0.f; position.scale =
//     TENTACLE_OXYGEN_SCALE;

//     // Set health bar
//     auto &health = registry.oxygen.emplace(entity);
//     oxygen.capacity = TENTACLE_OXYGEN;
//     oxygen.level = TENTACLE_OXYGEN;
//     oxygen.rate = 0.f;

//     // TODO: change to proper texture
//     registry.renderRequests.insert(
//         entity,
//         {TEXTURE_ASSET_ID::TEXTURE_COUNT,
//          EFFECT_ASSET_ID::TEXTURED,
//          GEOMETRY_BUFFER_ID::SPRITE});

//     return entity.operator unsigned int();
// }

// /////////////////////////////////////////////////////////////////
// // Serpent
// /////////////////////////////////////////////////////////////////

// /**
//  * @brief creates a health bar for a serpent
//  *
//  * @param renderer
//  * @param enemy - assumed to be a serpent
//  * @return entity id if successful, -1 otherwise
//  */
// int createSerpentHealthBar(RenderSystem *renderer, Entity enemy)
// {
//     // Check if enemy has a position component
//     if (!registry.positions.has(enemy))
//     {
//         std::cerr << "Error: Entity does not have a position component" <<
//         std::endl; return -1;
//     }

//     auto entity = Entity();

//     // Store a reference to the potentially re-used mesh object
//     Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
//     registry.meshPtrs.emplace(entity, &mesh);

//     // Get position of enemy
//     Position &enemyPos = registry.positions.get(enemy);

//     // Setting initial positon values
//     Position &position = registry.positions.emplace(entity);
//     position.position = enemyPos.position - vec2(0.f, enemyPos.scale.y / 2 +
//     ENEMY_O2_BAR_GAP); // TODO: guesstimate on where the HP should be, update
//     to proper position position.angle = 0.f; position.scale =
//     SERPENT_OXYGEN_SCALE;

//     // Set health bar
//     auto &health = registry.oxygen.emplace(entity);
//     oxygen.capacity = SERPENT_OXYGEN;
//     oxygen.level = SERPENT_OXYGEN;
//     oxygen.rate = 0.f;

//     // TODO: change to proper texture
//     registry.renderRequests.insert(
//         entity,
//         {TEXTURE_ASSET_ID::TEXTURE_COUNT,
//          EFFECT_ASSET_ID::TEXTURED,
//          GEOMETRY_BUFFER_ID::SPRITE});

//     return entity.operator unsigned int();
// }
