#include "enemy_factories.hpp"
#include "tiny_ecs_registry.hpp"

#include <iostream>

/////////////////////////////////////////////////////////////////
// Jellyfish
/////////////////////////////////////////////////////////////////
/**
 * @brief creates a jellyfish in a room
  // TODO: add the room as arg as well
 *
 * @param renderer
 * @param randPos - function that returns a random position in a room
 * @return entity id if successful, -1 otherwise
 */
Entity createJellyRoom(RenderSystem *renderer, vec2 (*randPos)(void))
{
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
Entity createJellyPos(RenderSystem *renderer, vec2 position)
{
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
  pos.scale = JELLY_SCALE_FACTOR * JELLY_BOUNDING_BOX;

  registry.renderRequests.insert(entity, {TEXTURE_ASSET_ID::JELLY,
                                          EFFECT_ASSET_ID::TEXTURED,
                                          GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}

/**
 * @brief creates a health bar for a jellyfish
 *
 * @param renderer
 * @param enemy - assumed to be a jellyfish
 * @return entity id if successful, -1 otherwise
 */
int createJellyHealthBar(RenderSystem *renderer, Entity enemy)
{
  // Check if enemy has a position component
  if (!registry.positions.has(enemy))
  {
    std::cerr << "Error: Entity does not have a position component" << std::endl;
    return -1;
  }

  auto entity = Entity();
  auto background = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);
  registry.meshPtrs.emplace(background, &mesh);

  // Get position of enemy
  Position &enemyPos = registry.positions.get(enemy);

  // Setting initial positon values
  Position &position = registry.positions.emplace(entity);
  position.position = enemyPos.position - vec2(0.f, enemyPos.scale.y / 2 + ENEMY_O2_BAR_GAP); // TODO: guesstimate on where the HP should be, update to proper position
  position.angle = 0.f;
  position.scale = JELLY_OXYGEN_SCALE;

  Position &backgroundPos = registry.positions.emplace(background);
  backgroundPos.position = position.position;
  backgroundPos.angle = 0.f;
  backgroundPos.scale = JELLY_OXYGEN_SCALE;

  // Set health bar
  auto &oxygen = registry.oxygen.emplace(entity);
  oxygen.capacity = JELLY_OXYGEN;
  oxygen.level = JELLY_OXYGEN;
  oxygen.rate = 0.f;

  // TODO: change to proper texture
  registry.renderRequests.insert(
      entity,
      {TEXTURE_ASSET_ID::TEXTURE_COUNT,
       EFFECT_ASSET_ID::TEXTURED,
       GEOMETRY_BUFFER_ID::SPRITE});

  registry.renderRequests.insert(
      background,
      {TEXTURE_ASSET_ID::TEXTURE_COUNT,
       EFFECT_ASSET_ID::TEXTURED,
       GEOMETRY_BUFFER_ID::SPRITE});

  return entity.operator unsigned int();
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
Entity createFishRoom(RenderSystem *renderer, vec2 (*randPos)(void))
{
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
Entity createFishPos(RenderSystem *renderer, vec2 position)
{
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
  pos.scale = FISH_SCALE_FACTOR * FISH_BOUNDING_BOX;

  // ai
  auto &wander = registry.wanders.emplace(entity);

  // TODO: add the room

  registry.renderRequests.insert(entity, {TEXTURE_ASSET_ID::FISH,
                                          EFFECT_ASSET_ID::TEXTURED,
                                          GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}

/**
 * @brief creates a health bar for a fish
 *
 * @param renderer
 * @param enemy - assumed to be a fish
 * @return entity id if successful, -1 otherwise
 */
int createFishHealthBar(RenderSystem *renderer, Entity enemy)
{
  // Check if enemy has a position component
  if (!registry.positions.has(enemy))
  {
    std::cerr << "Error: Entity does not have a position component" << std::endl;
    return -1;
  }

  auto entity = Entity();
  auto background = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);
  registry.meshPtrs.emplace(background, &mesh);

  // Get position of enemy
  Position &enemyPos = registry.positions.get(enemy);

  // Setting initial positon values
  Position &position = registry.positions.emplace(entity);
  position.position = enemyPos.position - vec2(0.f, enemyPos.scale.y / 2 + ENEMY_O2_BAR_GAP); // TODO: guesstimate on where the HP should be, update to proper position
  position.angle = 0.f;
  position.scale = FISH_OXYGEN_SCALE;

  Position &backgroundPos = registry.positions.emplace(background);
  backgroundPos.position = position.position;
  backgroundPos.angle = 0.f;
  backgroundPos.scale = FISH_OXYGEN_SCALE;

  // Set health bar
  auto &oxygen = registry.oxygen.emplace(entity);
  oxygen.capacity = FISH_OXYGEN;
  oxygen.level = FISH_OXYGEN;
  oxygen.rate = 0.f;

  // TODO: change to proper texture
  registry.renderRequests.insert(
      entity,
      {TEXTURE_ASSET_ID::TEXTURE_COUNT,
       EFFECT_ASSET_ID::TEXTURED,
       GEOMETRY_BUFFER_ID::SPRITE});

  registry.renderRequests.insert(
      background,
      {TEXTURE_ASSET_ID::TEXTURE_COUNT,
       EFFECT_ASSET_ID::TEXTURED,
       GEOMETRY_BUFFER_ID::SPRITE});

  return entity.operator unsigned int();
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
//         std::cerr << "Error: Entity does not have a position component" << std::endl;
//         return -1;
//     }

//     auto entity = Entity();

//     // Store a reference to the potentially re-used mesh object
//     Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
//     registry.meshPtrs.emplace(entity, &mesh);

//     // Get position of enemy
//     Position &enemyPos = registry.positions.get(enemy);

//     // Setting initial positon values
//     Position &position = registry.positions.emplace(entity);
//     position.position = enemyPos.position - vec2(0.f, enemyPos.scale.y / 2 + ENEMY_O2_BAR_GAP); // TODO: guesstimate on where the HP should be, update to proper position
//     position.angle = 0.f;
//     position.scale = SHARK_OXYGEN_SCALE;

//     // Set health bar
//     auto &oxygen = registry.oxygen.emplace(entity);
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
//         std::cerr << "Error: Entity does not have a position component" << std::endl;
//         return -1;
//     }

//     auto entity = Entity();

//     // Store a reference to the potentially re-used mesh object
//     Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
//     registry.meshPtrs.emplace(entity, &mesh);

//     // Get position of enemy
//     Position &enemyPos = registry.positions.get(enemy);

//     // Setting initial positon values
//     Position &position = registry.positions.emplace(entity);
//     position.position = enemyPos.position - vec2(0.f, enemyPos.scale.y / 2 + ENEMY_O2_BAR_GAP); // TODO: guesstimate on where the HP should be, update to proper position
//     position.angle = 0.f;
//     position.scale = OCTOPUS_OXYGEN_SCALE;

//     // Set health bar
//     auto &oxygen = registry.oxygen.emplace(entity);
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
//         std::cerr << "Error: Entity does not have a position component" << std::endl;
//         return -1;
//     }

//     auto entity = Entity();

//     // Store a reference to the potentially re-used mesh object
//     Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
//     registry.meshPtrs.emplace(entity, &mesh);

//     // Get position of enemy
//     Position &enemyPos = registry.positions.get(enemy);

//     // Setting initial positon values
//     Position &position = registry.positions.emplace(entity);
//     position.position = enemyPos.position - vec2(0.f, enemyPos.scale.y / 2 + ENEMY_O2_BAR_GAP); // TODO: guesstimate on where the HP should be, update to proper position
//     position.angle = 0.f;
//     position.scale = KRAB_OXYGEN_SCALE;

//     // Set health bar
//     auto &oxygen = registry.oxygen.emplace(entity);
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
//         std::cerr << "Error: Entity does not have a position component" << std::endl;
//         return -1;
//     }

//     auto entity = Entity();

//     // Store a reference to the potentially re-used mesh object
//     Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
//     registry.meshPtrs.emplace(entity, &mesh);

//     // Get position of enemy
//     Position &enemyPos = registry.positions.get(enemy);

//     // Setting initial positon values
//     Position &position = registry.positions.emplace(entity);
//     position.position = enemyPos.position - vec2(0.f, enemyPos.scale.y / 2 + ENEMY_O2_BAR_GAP); // TODO: guesstimate on where the HP should be, update to proper position
//     position.angle = 0.f;
//     position.scale = SEA_MINE_OXYGEN_SCALE;

//     // Set health bar
//     auto &oxygen = registry.oxygen.emplace(entity);
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
//         std::cerr << "Error: Entity does not have a position component" << std::endl;
//         return -1;
//     }

//     auto entity = Entity();

//     // Store a reference to the potentially re-used mesh object
//     Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
//     registry.meshPtrs.emplace(entity, &mesh);

//     // Get position of enemy
//     Position &enemyPos = registry.positions.get(enemy);

//     // Setting initial positon values
//     Position &position = registry.positions.emplace(entity);
//     position.position = enemyPos.position - vec2(0.f, enemyPos.scale.y / 2 + ENEMY_O2_BAR_GAP); // TODO: guesstimate on where the HP should be, update to proper position
//     position.angle = 0.f;
//     position.scale = MERPERSON_OXYGEN_SCALE;

//     // Set health bar
//     auto &oxygen = registry.oxygen.emplace(entity);
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
//         std::cerr << "Error: Entity does not have a position component" << std::endl;
//         return -1;
//     }

//     auto entity = Entity();

//     // Store a reference to the potentially re-used mesh object
//     Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
//     registry.meshPtrs.emplace(entity, &mesh);

//     // Get position of enemy
//     Position &enemyPos = registry.positions.get(enemy);

//     // Setting initial positon values
//     Position &position = registry.positions.emplace(entity);
//     position.position = enemyPos.position - vec2(0.f, enemyPos.scale.y / 2 + ENEMY_O2_BAR_GAP); // TODO: guesstimate on where the HP should be, update to proper position
//     position.angle = 0.f;
//     position.scale = TENTACLE_OXYGEN_SCALE;

//     // Set health bar
//     auto &oxygen = registry.oxygen.emplace(entity);
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
//         std::cerr << "Error: Entity does not have a position component" << std::endl;
//         return -1;
//     }

//     auto entity = Entity();

//     // Store a reference to the potentially re-used mesh object
//     Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
//     registry.meshPtrs.emplace(entity, &mesh);

//     // Get position of enemy
//     Position &enemyPos = registry.positions.get(enemy);

//     // Setting initial positon values
//     Position &position = registry.positions.emplace(entity);
//     position.position = enemyPos.position - vec2(0.f, enemyPos.scale.y / 2 + ENEMY_O2_BAR_GAP); // TODO: guesstimate on where the HP should be, update to proper position
//     position.angle = 0.f;
//     position.scale = SERPENT_OXYGEN_SCALE;

//     // Set health bar
//     auto &oxygen = registry.oxygen.emplace(entity);
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
