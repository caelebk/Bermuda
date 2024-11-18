#include "consumable_factories.hpp"
#include "map_factories.hpp"

#include <iostream>

#include "collision_util.hpp"
#include "items.hpp"
#include <player_hud.hpp>

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

  // Entities can't spawn in breakables
  for (Entity wall : registry.breakables.entities) {
    if (!registry.positions.has(wall)) {
      continue;
    }
    const Position wallPos = registry.positions.get(wall);
    if (box_collides(enemyPos, wallPos)) {
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
// Oxygen Canister
/////////////////////////////////////////////////////////////////
/**
 * @brief creates an oxygen canister at a specific position
 *
 * @param renderer
 * @param position
 * @return
 */
Entity createOxygenCanisterPos(RenderSystem* renderer, vec2 position, bool checkCollisions) {
  // Reserve an entity
  auto entity = Entity();

  auto& pos    = registry.positions.emplace(entity);
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = OXYGEN_CANISTER_SCALE_FACTOR * OXYGEN_CANISTER_BOUNDING_BOX;

  if (checkCollisions && !checkSpawnCollisions(entity)) {
    // returns invalid entity, since id's start from 1
    registry.remove_all_components_of(entity);
    return Entity(0);
  }
  
  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make consumable
  Consumable &c = registry.consumables.emplace(entity);
  c.respawnFn = respawnOxygenCanister;

  // Add stats
  auto& refill  = registry.oxygenModifiers.emplace(entity);
  refill.amount = OXYGEN_CANISTER_QTY;

  // physics and pos

  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::OXYGEN_CANISTER, EFFECT_ASSET_ID::TEXTURED,
               GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}

/////////////////////////////////////////////////////////////////
// Unlock the boss room.
/////////////////////////////////////////////////////////////////
Entity unlockBossDoors(RenderSystem* renderer, vec2 position,
                       bool checkCollisions) {
  for (auto& door_connection : registry.doorConnections.components) {
    door_connection.locked = false;
  }
  createGeyserPos(renderer, position, false);
  return Entity(0);
}

// TODO: remove asap
Entity unlockTutorial(RenderSystem* renderer, vec2 position,
                       bool checkCollisions) {
  for (auto& door_connection : registry.doorConnections.components) {
    door_connection.locked = false;
  }
  return Entity(0);
}

/////////////////////////////////////////////////////////////////
// Red Key
/////////////////////////////////////////////////////////////////
/**
 * @brief creates a red key at a specific position
 *
 * @param renderer
 * @param position
 * @return
 */

Entity createRedKeyPos(RenderSystem* renderer, vec2 position,
                       bool checkCollisions) {
  // Reserve an entity
  auto entity = Entity();

  float scale_reduction = 0.5f;
  auto& pos    = registry.positions.emplace(entity);
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = scale_reduction * RED_KEY_SCALE_FACTOR * RED_KEY_BOUNDING_BOX;

  if (checkCollisions && !checkSpawnCollisions(entity)) {
    // returns invalid entity, since id's start from 1
    registry.remove_all_components_of(entity);
    return Entity(0);
  }
  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make item
  Item &i = registry.items.emplace(entity);
  i.item = INVENTORY::RED_KEY;
  EntityState es;
  i.respawnFn = createRedKeyRespawnFn;

  // physics and pos
  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::RED_KEY, EFFECT_ASSET_ID::TEXTURED,
               GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}

Entity createRedKeyRespawnFn(RenderSystem *renderer, EntityState es) {
  if (registry.inventory.get(player).redKey) {
    return player; // the function does nothing with this but we need it to compile on VS, so hopefully it doesn't break.
  }
  Entity entity = createRedKeyPos(renderer, es.position.position, false);
  return entity;
}

/////////////////////////////////////////////////////////////////
// Blue Key
/////////////////////////////////////////////////////////////////
/**
 * @brief creates a blue key at a specific position
 *
 * @param renderer
 * @param position
 * @return
 */
Entity createBlueKeyPos(RenderSystem* renderer, vec2 position, bool checkCollisions) {
  // Reserve an entity
  auto entity = Entity();

  float scale_reduction = 0.5f;
  auto& pos    = registry.positions.emplace(entity);
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = scale_reduction * BLUE_KEY_SCALE_FACTOR * BLUE_KEY_BOUNDING_BOX;

  if (checkCollisions && !checkSpawnCollisions(entity)) {
    // returns invalid entity, since id's start from 1
    registry.remove_all_components_of(entity);
    return Entity(0);
  }
  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make consumable
  Item &i = registry.items.emplace(entity);
  i.item = INVENTORY::BLUE_KEY;
  EntityState es;
  i.respawnFn = createBlueKeyRespawnFn;

  // physics and pos
  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::BLUE_KEY, EFFECT_ASSET_ID::TEXTURED,
               GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}

Entity createBlueKeyRespawnFn(RenderSystem *renderer, EntityState es) {
  if (registry.inventory.get(player).blueKey) {
    return player; // the function does nothing with this but we need it to compile on VS, so hopefully it doesn't break.
  }
  Entity entity = createBlueKeyPos(renderer, es.position.position, false);
  return entity;
}

/////////////////////////////////////////////////////////////////
// Yellow Key
/////////////////////////////////////////////////////////////////
/**
 * @brief creates a yellow key at a specific position
 *
 * @param renderer
 * @param position
 * @return
 */
Entity createYellowKeyPos(RenderSystem* renderer, vec2 position, bool checkCollisions) {
  // Reserve an entity
  auto entity = Entity();

  float scale_reduction = 0.5f;
  auto& pos    = registry.positions.emplace(entity);
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = scale_reduction * YELLOW_KEY_SCALE_FACTOR * YELLOW_KEY_BOUNDING_BOX;

  if (checkCollisions && !checkSpawnCollisions(entity)) {
    // returns invalid entity, since id's start from 1
    registry.remove_all_components_of(entity);
    return Entity(0);
  }
  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make consumable
  Item &i = registry.items.emplace(entity);
  i.item = INVENTORY::YELLOW_KEY;
  EntityState es;
  i.respawnFn = createYellowKeyRespawnFn;

  // physics and pos
  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::YELLOW_KEY, EFFECT_ASSET_ID::TEXTURED,
               GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}

Entity createYellowKeyRespawnFn(RenderSystem *renderer, EntityState es) {
  if (registry.inventory.get(player).yellowKey) {
    return player; // the function does nothing with this but we need it to compile on VS, so hopefully it doesn't break.
  }
  Entity entity = createYellowKeyPos(renderer, es.position.position, false);
  return entity;
}

/**
 * @brief Respawns an oxygen canister based on its entity state
 *
 * @param renderer 
 * @param es 
 * @return 
 */
Entity respawnOxygenCanister(RenderSystem *renderer, EntityState es) {
  Entity entity = createOxygenCanisterPos(renderer, es.position.position, false);
  return entity;
}

/////////////////////////////////////////////////////////////////
// Nets
/////////////////////////////////////////////////////////////////
Entity createNetDropPos(RenderSystem* renderer, vec2 position,
                               bool checkCollisions) {
  // Reserve an entity
  auto entity = Entity();

  auto& pos    = registry.positions.emplace(entity);
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = NET_DROP_SCALE_FACTOR * NET_DROP_BOUNDING_BOX;

  if (checkCollisions && !checkSpawnCollisions(entity)) {
    // returns invalid entity, since id's start from 1
    registry.remove_all_components_of(entity);
    return Entity(0);
  }
  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make consumable
  Consumable& c = registry.consumables.emplace(entity);
  c.respawnFn   = respawnNetDrop;

  // Add stats
  auto& weapon_drop  = registry.weaponDrops.emplace(entity);
  weapon_drop.type = INVENTORY::NET;

  // physics and pos

  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::NET, EFFECT_ASSET_ID::TEXTURED,
               GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}

/**
 * @brief Respawns a Geyser based on its entity state
 *
 * @param renderer
 * @param es
 * @return
 */
Entity respawnNetDrop(RenderSystem* renderer, EntityState es) {
  Entity entity =
      createNetDropPos(renderer, es.position.position, false);
  return entity;
}

/////////////////////////////////////////////////////////////////
// Concussive
/////////////////////////////////////////////////////////////////
Entity createConcussiveDropPos(RenderSystem* renderer, vec2 position,
                        bool checkCollisions) {
  // Reserve an entity
  auto entity = Entity();

  auto& pos    = registry.positions.emplace(entity);
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = CONCUSSIVE_DROP_SCALE_FACTOR * CONCUSSIVE_DROP_BOUNDING_BOX;

  if (checkCollisions && !checkSpawnCollisions(entity)) {
    // returns invalid entity, since id's start from 1
    registry.remove_all_components_of(entity);
    return Entity(0);
  }
  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make consumable
  Consumable& c = registry.consumables.emplace(entity);
  c.respawnFn   = respawnConcussiveDrop;

  // Add stats
  auto& weapon_drop = registry.weaponDrops.emplace(entity);
  weapon_drop.type  = INVENTORY::CONCUSSIVE;

  // physics and pos

  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::CONCUSSIVE, EFFECT_ASSET_ID::TEXTURED,
               GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}

/**
 * @brief Respawns a Geyser based on its entity state
 *
 * @param renderer
 * @param es
 * @return
 */
Entity respawnConcussiveDrop(RenderSystem* renderer, EntityState es) {
  Entity entity = createConcussiveDropPos(renderer, es.position.position, false);
  return entity;
}

/////////////////////////////////////////////////////////////////
// Torpedo
/////////////////////////////////////////////////////////////////
Entity createTorpedoDropPos(RenderSystem* renderer, vec2 position,
                               bool checkCollisions) {
  // Reserve an entity
  auto entity = Entity();

  auto& pos    = registry.positions.emplace(entity);
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = TORPEDO_DROP_SCALE_FACTOR * TORPEDO_DROP_BOUNDING_BOX;

  if (checkCollisions && !checkSpawnCollisions(entity)) {
    // returns invalid entity, since id's start from 1
    registry.remove_all_components_of(entity);
    return Entity(0);
  }
  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make consumable
  Consumable& c = registry.consumables.emplace(entity);
  c.respawnFn   = respawnTorpedoDrop;

  // Add stats
  auto& weapon_drop = registry.weaponDrops.emplace(entity);
  weapon_drop.type  = INVENTORY::TORPEDO;

  // physics and pos

  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::TORPEDO, EFFECT_ASSET_ID::TEXTURED,
               GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}

/**
 * @brief Respawns a Geyser based on its entity state
 *
 * @param renderer
 * @param es
 * @return
 */
Entity respawnTorpedoDrop(RenderSystem* renderer, EntityState es) {
  Entity entity =
      createTorpedoDropPos(renderer, es.position.position, false);
  return entity;
}

/////////////////////////////////////////////////////////////////
// Shrimp
/////////////////////////////////////////////////////////////////
Entity createShrimpDropPos(RenderSystem* renderer, vec2 position,
                               bool checkCollisions) {
  // Reserve an entity
  auto entity = Entity();

  auto& pos    = registry.positions.emplace(entity);
  pos.angle    = 0.f;
  pos.position = position;
  pos.scale    = SHRIMP_DROP_SCALE_FACTOR * SHRIMP_DROP_BOUNDING_BOX;

  if (checkCollisions && !checkSpawnCollisions(entity)) {
    // returns invalid entity, since id's start from 1
    registry.remove_all_components_of(entity);
    return Entity(0);
  }
  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(entity, &mesh);

  // make consumable
  Consumable& c = registry.consumables.emplace(entity);
  c.respawnFn   = respawnShrimpDrop;

  // Add stats
  auto& weapon_drop = registry.weaponDrops.emplace(entity);
  weapon_drop.type  = INVENTORY::SHRIMP;

  // physics and pos

  registry.renderRequests.insert(
      entity, {TEXTURE_ASSET_ID::SHRIMP, EFFECT_ASSET_ID::TEXTURED,
               GEOMETRY_BUFFER_ID::SPRITE});

  return entity;
}

/**
 * @brief Respawns a Geyser based on its entity state
 *
 * @param renderer
 * @param es
 * @return
 */
Entity respawnShrimpDrop(RenderSystem* renderer, EntityState es) {
  Entity entity =
      createShrimpDropPos(renderer, es.position.position, false);
  return entity;
}



