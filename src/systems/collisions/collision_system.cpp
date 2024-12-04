#include "collision_system.hpp"

#include <consumable_utils.hpp>
#include <cstdio>
#include <damage.hpp>
#include <glm/geometric.hpp>
#include <physics_system.hpp>
#include <player_controls.hpp>
#include <player_factories.hpp>
#include <player_hud.hpp>

#include "ai.hpp"
#include "consumable_factories.hpp"
#include "debuff.hpp"
#include "enemy.hpp"
#include "entity_type.hpp"
#include "items.hpp"
#include "oxygen.hpp"
#include "player.hpp"
#include "tiny_ecs_registry.hpp"

void CollisionSystem::init(RenderSystem* renderer, LevelSystem* level) {
  this->renderer = renderer;
  this->level    = level;
}

bool CollisionSystem::checkBoxCollision(Entity entity_i, Entity entity_j) {
  if (!registry.positions.has(entity_i) || !registry.positions.has(entity_j)) {
    return false;
  }
  Position& position_i = registry.positions.get(entity_i);
  Position& position_j = registry.positions.get(entity_j);
  if (box_collides(position_i, position_j)) {
    registry.collisions.emplace_with_duplicates(entity_i, entity_j);
    registry.collisions.emplace_with_duplicates(entity_j, entity_i);
    return true;
  }
  return false;
}

bool CollisionSystem::checkPlayerMeshCollision(Entity entity_i, Entity entity_j,
                                               Entity collisionMesh) {
  if (!registry.positions.has(entity_i) || !registry.positions.has(entity_j)) {
    return false;
  }
  Position& position_i = registry.positions.get(entity_i);
  Position& position_j = registry.positions.get(entity_j);
  bool      player_bb_collides;
  if (registry.enemyProjectiles.has(entity_j) &&
      registry.enemyProjectiles.get(entity_j).type == ENTITY_TYPE::SHOCKWAVE) {
    // shockwave uses circle mesh collision
    float radius       = max(position_j.scale.x, position_j.scale.y) / 2;
    player_bb_collides = circle_box_collides(position_j, radius, position_i);
  } else {
    player_bb_collides = box_collides(position_i, position_j);
  }

  if (player_bb_collides && mesh_collides(collisionMesh, entity_j)) {
    registry.collisions.emplace_with_duplicates(entity_i, entity_j);
    registry.collisions.emplace_with_duplicates(entity_j, entity_i);
    return true;
  }
  return false;
}

bool CollisionSystem::checkCircleCollision(Entity entity_i, Entity entity_j) {
  if (!registry.positions.has(entity_i) || !registry.positions.has(entity_j)) {
    return false;
  }
  Position& position_i = registry.positions.get(entity_i);
  Position& position_j = registry.positions.get(entity_j);
  if (circle_collides(position_i, position_j)) {
    registry.collisions.emplace_with_duplicates(entity_i, entity_j);
    registry.collisions.emplace_with_duplicates(entity_j, entity_i);
    return true;
  }
  return false;
}

bool CollisionSystem::checkCircleBoxCollision(Entity circle_bound_entity,
                                              Entity box_bound_entity) {
  if (!registry.positions.has(circle_bound_entity) ||
      !registry.positions.has(box_bound_entity)) {
    return false;
  }
  Position& position_i = registry.positions.get(circle_bound_entity);
  float     radius     = max(position_i.scale.x, position_i.scale.y) / 2.f;
  Position& position_j = registry.positions.get(box_bound_entity);
  if (circle_box_collides(position_i, radius, position_j)) {
    registry.collisions.emplace_with_duplicates(circle_bound_entity,
                                                box_bound_entity);
    registry.collisions.emplace_with_duplicates(box_bound_entity,
                                                circle_bound_entity);
    return true;
  }
  return false;
}

void CollisionSystem::step(float elapsed_ms) {
  collision_detection();

  handle_collision_end();

  collision_resolution();
}

// Check if collision has ended here.
void CollisionSystem::handle_collision_end() {
  // Check if pressure plate collision has ended.
  for (Entity entity : registry.pressurePlates.entities) {
    bool pressurePlateCollisionExists = registry.collisions.has(entity);
    if (!pressurePlateCollisionExists) {
      if (!registry.sounds.has(entity) &&
          registry.pressurePlates.get(entity).active) {
        registry.sounds.insert(entity, Sound(SOUND_ASSET_ID::PRESSURE_PLATE));
      }
      registry.pressurePlates.get(entity).active = false;
      // Connect to an available door, if we haven't yet.
      // Exploits the fact that there's only 1 PP per room.
      for (Entity& entity : registry.activeDoors.entities) {
        if (registry.doorConnections.has(entity)) {
          DoorConnection& door_connection = registry.doorConnections.get(entity);
          if (door_connection.objective == Objective::PRESSURE_PLATE) {
            door_connection.locked = true;

            // change the sprite
            if (registry.renderRequests.has(entity)) {
              registry.renderRequests.remove(entity);

              level->assign_door_sprite(entity, door_connection);
            }
          }
        }
      }
      registry.renderRequests.get(entity).used_texture =
          TEXTURE_ASSET_ID::PRESSURE_PLATE_OFF;
    }
  }

}

/***********************************
Collision Detection (has precedence noted below)
************************************/
void CollisionSystem::collision_detection() {
  // 1. Detect player projectile collisions
  detectPlayerProjectileCollisions();

  // 2. Detect player collisions
  detectPlayerCollisions();

  // 3. Detect enemy support collisions
  detectEnemySupportCollisions();

  // 4. Detect mass collisions
  detectMassCollisions();

  // 5. Detect wall collisions
  detectWallCollisions();

  // 6. Detect door collisions
  detectDoorCollisions();
}

void CollisionSystem::detectPlayerProjectileCollisions() {
  ComponentContainer<PlayerProjectile>& playerproj_container =
      registry.playerProjectiles;
  ComponentContainer<Deadly>&     enemy_container      = registry.deadlys;
  ComponentContainer<ActiveWall>& wall_container       = registry.activeWalls;
  ComponentContainer<Consumable>& consumable_container = registry.consumables;

  for (uint i = 0; i < playerproj_container.components.size(); i++) {
    Entity entity_i = playerproj_container.entities[i];
    if (!registry.positions.has(entity_i) ||
        registry.playerProjectiles.get(entity_i).is_loaded) {
      continue;
    }

    // detect player projectile and wall collisions
    for (uint j = 0; j < wall_container.size(); j++) {
      Entity entity_j = wall_container.entities[j];
      checkBoxCollision(entity_i, entity_j);
    }

    // detect player projectile and oxygen canister collisions
    for (uint j = 0; j < consumable_container.size(); j++) {
      Entity      entity_j   = consumable_container.entities[j];
      Consumable& consumable = consumable_container.get(entity_j);
      if (consumable.type != ENTITY_TYPE::OXYGEN_CANISTER) {
        continue;
      }
      checkBoxCollision(entity_i, entity_j);
    }

    // detect player projectile and enemy collisions
    for (uint j = 0; j < enemy_container.size(); j++) {
      Entity entity_j = enemy_container.entities[j];
      bool   collided = checkCircleCollision(entity_i, entity_j);
      // if the projectile is single target and collided, don't check for
      // anymore enemies.
      PlayerProjectile& playerproj_comp = playerproj_container.components[i];
      if (collided && (playerproj_comp.type == PROJECTILES::HARPOON ||
                       playerproj_comp.type == PROJECTILES::NET ||
                       playerproj_comp.type == PROJECTILES::TORPEDO)) {
        break;
      }
    }
  }
}

void CollisionSystem::detectPlayerCollisions() {
  ComponentContainer<Player>&         player_container = registry.players;
  ComponentContainer<Deadly>&         enemy_container  = registry.deadlys;
  ComponentContainer<EnemyProjectile> enemy_proj_container =
      registry.enemyProjectiles;
  ComponentContainer<Item>&         item_container       = registry.items;
  ComponentContainer<Consumable>&   consumable_container = registry.consumables;
  ComponentContainer<Interactable>& interactable_container =
      registry.interactable;

  for (uint i = 0; i < player_container.components.size(); i++) {
    Entity entity_i = player_container.entities[i];
    if (!registry.positions.has(entity_i)) {
      continue;
    }
    Player player_comp = registry.players.get(entity_i);

    // detect player and enemy collisions
    for (uint j = 0; j < enemy_container.size(); j++) {
      Entity entity_j = enemy_container.entities[j];
      // don't detect the enemy collision if their attack is on cooldown
      if (registry.modifyOxygenCd.has(entity_j)) {
        ModifyOxygenCD& modifyOxygenCd = registry.modifyOxygenCd.get(entity_j);
        if (modifyOxygenCd.curr_cd > 0.f) {
          continue;
        }
      }
      checkPlayerMeshCollision(entity_i, entity_j, player_comp.collisionMesh);
    }

    for (uint j = 0; j < enemy_proj_container.size(); j++) {
      Entity entity_j = enemy_proj_container.entities[j];
      checkPlayerMeshCollision(entity_i, entity_j, player_comp.collisionMesh);
    }

    for (uint j = 0; j < item_container.size(); j++) {
      Entity entity_j = item_container.entities[j];
      checkPlayerMeshCollision(entity_i, entity_j, player_comp.collisionMesh);
    }

    for (uint j = 0; j < consumable_container.size(); j++) {
      Entity entity_j = consumable_container.entities[j];
      checkPlayerMeshCollision(entity_i, entity_j, player_comp.collisionMesh);
    }

    for (uint j = 0; j < interactable_container.size(); j++) {
      Entity entity_j = interactable_container.entities[j];
      // don't detect the interactable collision if their attack is on cooldown
      if (registry.modifyOxygenCd.has(entity_j)) {
        ModifyOxygenCD& modifyOxygenCd = registry.modifyOxygenCd.get(entity_j);
        if (modifyOxygenCd.curr_cd > 0.f) {
          continue;
        }
      }
      checkPlayerMeshCollision(entity_i, entity_j, player_comp.collisionMesh);
    }
  }
}

void CollisionSystem::detectEnemySupportCollisions() {
  ComponentContainer<Deadly>&      enemy_container = registry.deadlys;
  ComponentContainer<EnemySupport> enemy_supp_container =
      registry.enemySupports;

  for (uint i = 0; i < enemy_supp_container.components.size(); i++) {
    Entity entity_i = enemy_supp_container.entities[i];
    if (!registry.positions.has(entity_i)) {
      continue;
    }

    for (uint j = 0; j < enemy_container.size(); j++) {
      Entity entity_j = enemy_container.entities[j];
      if (registry.enemySupports.get(entity_i).ignores_user &&
          entity_j == registry.enemySupports.get(entity_i).user)
        continue;
      if (!registry.oxygen.has(entity_j)) continue;
      Oxygen& entity_j_oxygen = registry.oxygen.get(entity_j);
      if (entity_j_oxygen.level >= entity_j_oxygen.capacity) continue;
      checkCircleBoxCollision(entity_i, entity_j);
    }
  }
}

void CollisionSystem::detectWallCollisions() {
  ComponentContainer<Deadly>&         enemy_container = registry.deadlys;
  ComponentContainer<EnemyProjectile> enemy_proj_container =
      registry.enemyProjectiles;
  ComponentContainer<EnemySupport> enemy_supp_container =
      registry.enemySupports;
  ComponentContainer<ActiveWall>& wall_container = registry.activeWalls;

  for (uint i = 0; i < wall_container.components.size(); i++) {
    Entity entity_i = wall_container.entities[i];
    if (!registry.positions.has(entity_i)) {
      continue;
    }

    for (uint j = 0; j < enemy_container.size(); j++) {
      Entity entity_j = enemy_container.entities[j];
      checkBoxCollision(entity_i, entity_j);
    }

    for (uint j = 0; j < enemy_proj_container.size(); j++) {
      Entity entity_j = enemy_proj_container.entities[j];
      checkBoxCollision(entity_i, entity_j);
    }

    for (uint j = 0; j < enemy_supp_container.size(); j++) {
      Entity entity_j = enemy_supp_container.entities[j];
      checkCircleBoxCollision(entity_j, entity_i);
    }
  }
}

void CollisionSystem::detectDoorCollisions() {
  ComponentContainer<ActiveDoor>& door_container   = registry.activeDoors;
  ComponentContainer<Deadly>&     enemy_container  = registry.deadlys;
  ComponentContainer<Player>&     player_container = registry.players;

  for (uint i = 0; i < door_container.components.size(); i++) {
    Entity entity_i = door_container.entities[i];

    for (uint j = 0; j < enemy_container.size(); j++) {
      Entity entity_j = enemy_container.entities[j];
      checkBoxCollision(entity_i, entity_j);
    }

    for (uint j = 0; j < player_container.size(); j++) {
      Entity  entity_j    = player_container.entities[j];
      Player& player_comp = registry.players.get(entity_j);
      checkPlayerMeshCollision(entity_j, entity_i, player_comp.collisionMesh);
    }
  }
}

void CollisionSystem::detectMassCollisions() {
  ComponentContainer<Mass>&         mass_container = registry.masses;
  ComponentContainer<Interactable>& interactable_container =
      registry.interactable;
  ComponentContainer<ActiveWall>& wall_container = registry.activeWalls;

  for (uint i = 0; i < mass_container.size(); i++) {
    Entity entity_i = mass_container.entities[i];

    for (uint j = 0; j < interactable_container.size(); j++) {
      Entity entity_j = interactable_container.entities[j];
      if (registry.players.has(entity_i)) {
        Player& player_comp = registry.players.get(entity_i);
        checkPlayerMeshCollision(entity_i, entity_j, player_comp.collisionMesh);
      } else {
        checkBoxCollision(entity_i, entity_j);
      }
    }

    for (uint j = 0; j < wall_container.size(); j++) {
      Entity entity_j = wall_container.entities[j];
      if (entity_i == entity_j) {
        continue;
      }
      if (registry.players.has(entity_i)) {
        Player& player_comp = registry.players.get(entity_i);
        checkPlayerMeshCollision(entity_i, entity_j, player_comp.collisionMesh);
      } else {
        checkBoxCollision(entity_i, entity_j);
      }
    }
  }
}

void CollisionSystem::collision_resolution_debug_info(Entity entity,
                                                      Entity entity_other) {
  printf("Entity:\n");
  registry.list_all_components_of(entity);
  printf("Entity Other:\n");
  registry.list_all_components_of(entity_other);
}

void CollisionSystem::collision_resolution() {
  auto& collisionsRegistry = registry.collisions;
  // printf("Collisions size: %d\n", collisionsRegistry.components.size());
  for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
    Entity entity       = collisionsRegistry.entities[i];
    Entity entity_other = collisionsRegistry.components[i].other;

    // collision_resolution_debug_info(entity, entity_other);

    // Player Collision Handling
    if (registry.players.has(entity)) {
      routePlayerCollisions(entity, entity_other);
    }

    // Wall Collision Handling
    if (registry.activeWalls.has(entity)) {
      routeWallCollisions(entity, entity_other);
    }

    // Door Collision Handling
    if (registry.activeDoors.has(entity)) {
      routeDoorCollisions(entity, entity_other);
    }

    // Enemy Collision Handling
    if (registry.deadlys.has(entity)) {
      routeEnemyCollisions(entity, entity_other);
    }

    // Player Projectile Collision Handling
    if (registry.playerProjectiles.has(entity)) {
      routePlayerProjCollisions(entity, entity_other);
    }

    // Player Item Collision Handling
    if (registry.items.has(entity)) {
      routeItemCollisions(entity, entity_other);
    }

    // Consumable Collision Handling
    if (registry.consumables.has(entity)) {
      routeConsumableCollisions(entity, entity_other);
    }

    // Interactable Collision Handling
    if (registry.interactable.has(entity)) {
      routeInteractableCollisions(entity, entity_other);
    }
  }
  // Remove all collisions from this simulation step
  registry.collisions.clear();
}

/*********************************************
  Entity -> Other Entity Collision Routing
**********************************************/
void CollisionSystem::routePlayerCollisions(Entity player, Entity other) {
  if (registry.deadlys.has(other)) {
    resolvePlayerEnemyCollision(player, other);
  }
  if (registry.enemyProjectiles.has(other)) {
    resolvePlayerEnemyProjCollision(player, other);
  }
  if (registry.items.has(other)) {
    resolvePlayerItemCollision(player, other);
  }
  if (registry.consumables.has(other)) {
    resolvePlayerConsumableCollision(player, other);
  }
  if (registry.activeWalls.has(other)) {
    resolveStopOnWall(other, player);
  }
  if (registry.activeDoors.has(other)) {
    resolveDoorPlayerCollision(other, player);
  }
  if (registry.interactable.has(other)) {
    resolvePlayerInteractableCollision(player, other);
  }
}

void CollisionSystem::routeEnemyCollisions(Entity enemy, Entity other) {
  bool routed = false;
  if (registry.players.has(other)) {
    routed = true;
    resolvePlayerEnemyCollision(other, enemy);
  }
  if (registry.playerProjectiles.has(other)) {
    routed = true;
    // resolveEnemyPlayerProjCollision(enemy, other);
  }
  if (registry.activeWalls.has(other)) {
    resolveWallEnemyCollision(other, enemy);
    routed = true;
  }
  if (registry.enemySupports.has(other)) {
    resolveEnemyEnemySupportCollision(enemy, other);
    routed = true;
  }

  // if an enemy is acting as a projectile and hits something, it
  // is no longer acting as a projectile and goes back to its regular ai
  if (routed && registry.actsAsProjectile.has(enemy)) {
    registry.actsAsProjectile.remove(enemy);
  }
}

void CollisionSystem::routeWallCollisions(Entity wall, Entity other) {
  if (!registry.motions.has(other)) {
    return;
  }

  if (registry.players.has(other)) {
    if (registry.masses.has(wall) && registry.masses.has(other)) {
      resolveMassCollision(wall, other);
    } else {
      resolveStopOnWall(wall, other);
    }
  }
  if (registry.activeWalls.has(other)) {
    if (registry.masses.has(wall) && registry.masses.has(other)) {
      resolveMassCollision(wall, other);
    } else {
      resolveStopOnWall(wall, other);
    }
  }
  if (registry.playerProjectiles.has(other)) {
    resolveWallPlayerProjCollision(wall, other);

    // if (registry.breakables.has(wall)) {
    //   resolveBreakablePlayerProjCollision(wall, other);
    // }
  }
  if (registry.enemyProjectiles.has(other)) {
    ENTITY_TYPE type = registry.enemyProjectiles.get(other).type;
    // what is a wall collision rahhhhh
    if (type == ENTITY_TYPE::SHOCKWAVE) {
      return;
    }

    if (registry.breakables.has(wall)) {
      // too cool an attack to be stopped by a wall
      if (type == ENTITY_TYPE::FIREBALL || type == ENTITY_TYPE::RAGE_PROJ) {
        return;
      }
      resolveBreakableEnemyProjCollision(wall, other);
    } else {
      resolveWallEnemyProjCollision(wall, other);
    }
  }
  if (registry.enemySupports.has(other)) {
    resolveWallEnemyProjCollision(wall, other);
  }
}

void CollisionSystem::routeDoorCollisions(Entity door, Entity other) {
  if (!registry.motions.has(other)) {
    return;
  }

  if (registry.players.has(other)) {
    resolveDoorPlayerCollision(door, other);
  }

  // Since enemies and projectiles can't enter different rooms, simply treat
  // their collisions like a wall.
  if (registry.playerProjectiles.has(other)) {
    resolveWallPlayerProjCollision(door, other);
  }
  if (registry.deadlys.has(other)) {
    resolveWallEnemyCollision(door, other);
  }
  if (registry.enemyProjectiles.has(other)) {
    resolveWallEnemyProjCollision(door, other);
  }
  // same resolution as above, avoid "or" cond. in case this ever changes
  if (registry.enemySupports.has(other)) {
    resolveWallEnemyProjCollision(door, other);
  }
}

void CollisionSystem::routePlayerProjCollisions(Entity player_proj,
                                                Entity other) {
  if (registry.deadlys.has(other)) {
    resolveEnemyPlayerProjCollision(other, player_proj);
  }
  if (registry.activeWalls.has(other)) {
    resolveWallPlayerProjCollision(other, player_proj);
  }
  if (registry.breakables.has(other)) {
    resolveBreakablePlayerProjCollision(other, player_proj);
  }
  if (registry.consumables.has(other)) {
    resolveCanisterPlayerProjCollision(other, player_proj);
  }

  PlayerProjectile& player_proj_component =
      registry.playerProjectiles.get(player_proj);
  bool checkWepSwapped = player_proj != player_projectile;

  // Remove render projectile if weapons have been swapped or collision just
  // occured, except for concussive (handled in debuff.cpp) & shrimp (handled in
  // resolveWallPlayerProj)
  if (checkWepSwapped &&
      player_proj_component.type != PROJECTILES::CONCUSSIVE &&
      player_proj_component.type != PROJECTILES::SHRIMP) {
    destroyGunOrProjectile(player_proj);
  }
}

void CollisionSystem::routeItemCollisions(Entity item, Entity other) {
  if (registry.players.has(other)) {
    resolvePlayerItemCollision(other, item);
  }
}

void CollisionSystem::routeConsumableCollisions(Entity consumable,
                                                Entity other) {
  if (registry.players.has(other)) {
    resolvePlayerConsumableCollision(other, consumable);
  }
}

void CollisionSystem::routeInteractableCollisions(Entity interactable,
                                                  Entity other) {
  if (registry.players.has(other)) {
    resolvePlayerInteractableCollision(other, interactable);
  }

  if (registry.masses.has(other)) {
    resolveMassInteractableCollision(other, interactable);
  }
}

/*********************************************
    Entity <-> Entity Collision Resolutions
**********************************************/
void CollisionSystem::resolvePlayerEnemyCollision(Entity player, Entity enemy) {
  if (!registry.oxygenModifiers.has(enemy) && registry.shooters.has(enemy)) {
    // shooters without oxygenModifier don't hurt player on touch
    return;
  }
  handle_debuffs(player, enemy);
  addDamageIndicatorTimer(player);
  modifyOxygen(player, enemy);
  if (registry.deadlys.get(enemy).type == ENTITY_TYPE::TURTLE) {
    modifyOxygenAmount(enemy, -registry.oxygenModifiers.get(enemy).amount);
    if (!registry.sounds.has(enemy)) {
      registry.sounds.insert(enemy, SOUND_ASSET_ID::TURTLE);
    }
    addDamageIndicatorTimer(enemy);
  }
}

void CollisionSystem::resolvePlayerItemCollision(Entity player, Entity item) {
  if (registry.deathTimers.has(player)) {
    return;
  }

  // will add a key if this is in fact one
  if (registry.items.has(item)) {
    Item&     i     = registry.items.get(item);
    Objective color = i.item;

    // We need to route this somewhere because the collect functions take a
    // renderer and this class doesn't have one. Probably doesn't belong in
    // LevelSystem, but it's 2:06 am.
    level->collect_key(i.item);
  }
  registry.remove_all_components_of(item);
}

void CollisionSystem::resolvePlayerEnemyProjCollision(Entity player,
                                                      Entity enemy_proj) {
  // For now it's almost equal to the above, but make a new function just to
  // open it to changes

  EnemyProjectile& proj = registry.enemyProjectiles.get(enemy_proj);
  if (proj.type == ENTITY_TYPE::SHOCKWAVE &&
      !can_see_entity(registry.positions.get(enemy_proj),
                      registry.positions.get(player))) {
    // shockwave does not damage player if something blocks line of sight
    return;
  } else if (proj.type == ENTITY_TYPE::RAGE_PROJ) {
    // rage proj heals cthulhu on hit
    if (registry.bosses.entities.size() == 1) {
      modifyOxygenAmount(registry.bosses.entities[0],
                         -CTHULHU_RAGE_PROJ_DAMAGE);
    }
  }

  handle_debuffs(player, enemy_proj);
  addDamageIndicatorTimer(player);
  modifyOxygen(player, enemy_proj);

  // canisters "explode" to hurt player in aoe,
  // but since this is a direct hit, no explosion needed
  // but for consistency show the effects
  if (registry.enemyProjectiles.get(enemy_proj).type ==
      ENTITY_TYPE::OXYGEN_CANISTER) {
    registry.sounds.insert(Entity(), Sound(SOUND_ASSET_ID::EXPLOSION));
    make_canister_explosion(renderer,
                            registry.positions.get(enemy_proj).position);
  }
  // Assume the projectile should poof upon impact
  registry.remove_all_components_of(enemy_proj);
}

void CollisionSystem::resolvePlayerConsumableCollision(Entity player,
                                                       Entity consumable) {
  handle_consumable_collisions(player, consumable, renderer);
}

void CollisionSystem::resolvePlayerInteractableCollision(Entity player,
                                                         Entity interactable) {
  if (registry.deathTimers.has(player)) {
    return;
  }
  // TODO: add more affects M2+
  if (registry.pressurePlates.has(interactable)) {
    if (!registry.pressurePlates.get(interactable).active) {
      registry.renderRequests.get(interactable).used_texture =
          TEXTURE_ASSET_ID::PRESSURE_PLATE_ON;
      registry.pressurePlates.get(interactable).active = true;
      for (Entity& entity : registry.activeDoors.entities) {
        if (registry.doorConnections.has(entity)) {
          DoorConnection& door_connection = registry.doorConnections.get(entity);
          if (door_connection.objective == Objective::PRESSURE_PLATE) {
            door_connection.locked = false;

            // change the sprite
            if (registry.renderRequests.has(entity)) {
              registry.renderRequests.remove(entity);

              level->assign_door_sprite(entity, door_connection);
            }
          }
        }
      }
      if (!registry.sounds.has(interactable)) {
        registry.sounds.insert(interactable,
                               Sound(SOUND_ASSET_ID::PRESSURE_PLATE));
      }
    }
  }
  // will add oxygen to the player if it exists
  modifyOxygen(player, interactable);
}

void CollisionSystem::resolveEnemyPlayerProjCollision(Entity enemy,
                                                      Entity player_proj) {
  PlayerProjectile& playerproj_comp =
      registry.playerProjectiles.get(player_proj);

  if (!registry.motions.has(player_proj)) {
    return;
  }
  Motion& playerproj_motion = registry.motions.get(player_proj);

  // cthulhu takes no damage in transition, cannot be stunned
  bool is_cthulhu = registry.bosses.has(enemy) &&
      registry.bosses.get(enemy).type == ENTITY_TYPE::CTHULHU;
  if (!is_cthulhu || !registry.bosses.get(enemy).in_transition) {
    modifyOxygen(enemy, player_proj);
  }

  switch (playerproj_comp.type) {
    case PROJECTILES::HARPOON:
      if (!registry.sounds.has(player_proj)) {
        registry.sounds.insert(player_proj, Sound(SOUND_ASSET_ID::HITMARKER));
      }
      break;
    case PROJECTILES::NET:
      if (!is_cthulhu) {
        handle_debuffs(enemy, player_proj);
      }
      break;
    case PROJECTILES::CONCUSSIVE:
      // ignore boxes and jellyfish.
      if (!registry.activeWalls.has(enemy) && registry.motions.has(enemy)) {
        handle_debuffs(enemy, player_proj);
      }
      break;
    case PROJECTILES::TORPEDO:
      detectAndResolveExplosion(player_proj, enemy);
      makeTorpedoExplosion(renderer,
                           registry.positions.get(player_proj).position);
      break;
    case PROJECTILES::SHRIMP:
      /*detectAndResolveConeAOE(player_proj, enemy, SHRIMP_DAMAGE_ANGLE);*/
      // shrimps dont pierce if they hit a boss
      if (registry.bosses.has(enemy)) {
        Inventory& inventory = registry.inventory.get(player);
        bool check_wep_swap = player_projectile != player_proj;

        playerproj_motion.velocity = vec2(0.0f, 0.0f);
        playerproj_comp.is_loaded = true;

        if (check_wep_swap) {
          destroyGunOrProjectile(player_proj);
        }
        if (inventory.shrimp <= 0) {
          doWeaponSwap(harpoon, harpoon_gun, PROJECTILES::HARPOON);
          changeSelectedCounterColour(INVENTORY::HARPOON);
        }
      }
      break;
  }

  addDamageIndicatorTimer(enemy);

  // make enemies that track the player briefly start tracking them regardless
  // of range
  if (registry.trackPlayer.has(enemy)) {
    TracksPlayer& tracks = registry.trackPlayer.get(enemy);
    tracks.active_track  = true;
  }

  if (registry.bosses.has(enemy)) {
    Boss& boss = registry.bosses.get(enemy);
    // if sharkman hit, instantly charge at player
    if (boss.type == ENTITY_TYPE::SHARKMAN) {
      if (!registry.trackPlayer.has(enemy)) {
        boss.curr_cd = SHARKMAN_AI_CD;
        removeFromAI(enemy);
        addSharkmanTarget();
      }
      choose_new_direction(enemy, player_proj);
    }
  }
  
  if (playerproj_comp.type != PROJECTILES::CONCUSSIVE &&
      playerproj_comp.type != PROJECTILES::SHRIMP) {
    playerproj_motion.velocity = vec2(0.0f, 0.0f);
    playerproj_comp.is_loaded  = true;
  }
}

void CollisionSystem::resolveEnemyEnemySupportCollision(Entity enemy,
                                                        Entity enemy_support) {
  // addDamageIndicatorTimer(enemy);
  modifyOxygen(enemy, enemy_support);
  if (!registry.sounds.has(enemy)) {
    registry.sounds.insert(enemy, SOUND_ASSET_ID::SIREN);
  }

  // Assume the support entity should get thanos'd upon impact
  registry.remove_all_components_of(enemy_support);
}

void CollisionSystem::resolveBreakablePlayerProjCollision(Entity breakable,
                                                          Entity player_proj) {
  if (!registry.motions.has(player_proj)) {
    return;
  }

  PlayerProjectile& playerproj_comp =
      registry.playerProjectiles.get(player_proj);

  modifyOxygen(breakable, player_proj);

  if (playerproj_comp.type == PROJECTILES::TORPEDO) {
    detectAndResolveExplosion(player_proj, breakable);
    makeTorpedoExplosion(renderer,
                         registry.positions.get(player_proj).position);
  }
}

void CollisionSystem::resolveBreakableEnemyProjCollision(Entity breakable,
                                                         Entity enemy_proj) {
  if (!registry.motions.has(enemy_proj)) {
    return;
  }

  modifyOxygen(breakable, enemy_proj);

  // canister projectiles explode on walls
  if (registry.enemyProjectiles.has(enemy_proj) &&
      registry.enemyProjectiles.get(enemy_proj).type ==
          ENTITY_TYPE::OXYGEN_CANISTER) {
    detectAndResolveExplosion(enemy_proj, breakable);
    make_canister_explosion(renderer,
                            registry.positions.get(enemy_proj).position);
  }
  registry.remove_all_components_of(enemy_proj);
}

void CollisionSystem::resolveCanisterPlayerProjCollision(Entity canister,
                                                         Entity player_proj) {
  // hack, convert the canister's oxygen quantity to be damage instead of
  // healing
  OxygenModifier& oxygen = registry.oxygenModifiers.get(canister);
  oxygen.amount          = OXYGEN_CANISTER_DAMAGE;
  detectAndResolveExplosion(canister, player_proj);
  make_canister_explosion(renderer, registry.positions.get(canister).position);
  registry.remove_all_components_of(canister);

  if (registry.playerProjectiles.has(player_proj)) {
    PlayerProjectile& playerproj_comp =
        registry.playerProjectiles.get(player_proj);
    playerproj_comp.is_loaded = true;
  }
}

// hit_entity is the entity that got hit
void CollisionSystem::detectAndResolveExplosion(Entity proj,
                                                Entity hit_entity) {
  if (!registry.sounds.has(proj)) {
    registry.sounds.insert(Entity(), Sound(SOUND_ASSET_ID::EXPLOSION));
  }

  bool is_canister =
      (registry.enemyProjectiles.has(proj) &&
       registry.enemyProjectiles.get(proj).type ==
           ENTITY_TYPE::OXYGEN_CANISTER) ||
      (registry.consumables.has(proj) &&
       registry.consumables.get(proj).type == ENTITY_TYPE::OXYGEN_CANISTER);

  Position&     playerproj_position = registry.positions.get(proj);
  AreaOfEffect& playerproj_aoe      = registry.aoe.get(proj);

  // canister projectiles cannot hurt enemies/breakables,
  // prevents cthulhu from killing itself and tentacles
  if (!is_canister || registry.consumables.has(proj)) {
    for (Entity enemy_check : registry.deadlys.entities) {
      if (enemy_check == hit_entity || !registry.positions.has(hit_entity)) {
        continue;
      }
      Position& enemy_position = registry.positions.get(enemy_check);

      if (circle_box_collides(playerproj_position, playerproj_aoe.radius,
                              enemy_position)) {
        // cthulhu cannot take damage in transition
        if (!registry.bosses.has(enemy_check) ||
            registry.bosses.get(enemy_check).type != ENTITY_TYPE::CTHULHU ||
            !registry.bosses.get(enemy_check).in_transition) {
          modifyOxygen(enemy_check, proj);
          addDamageIndicatorTimer(enemy_check);
        }
      }
    }

    for (Entity breakable_check : registry.breakables.entities) {
      if (breakable_check == hit_entity ||
          !registry.positions.has(hit_entity)) {
        continue;
      }
      Position& enemy_position = registry.positions.get(breakable_check);

      if (circle_box_collides(playerproj_position, playerproj_aoe.radius,
                              enemy_position)) {
        modifyOxygen(breakable_check, proj);
        addDamageIndicatorTimer(breakable_check);
      }
    }
  }
  // canister explosions hurt the player
  if (is_canister && registry.positions.has(player)) {
    Position& player_position = registry.positions.get(player);

    if (circle_box_collides(playerproj_position, playerproj_aoe.radius,
                            player_position) &&
        mesh_collides(registry.players.get(player).collisionMesh, proj)) {
      modifyOxygen(player, proj);
      addDamageIndicatorTimer(player);
    }
  }

  // NOTE: experimental
  for (Entity canister_check : registry.consumables.entities) {
    if (!registry.consumables.has(canister_check)) {
      // since we remove as we iterate, sometimes it might not be inside the
      // registry anymore
      continue;
    }

    Consumable& consumable = registry.consumables.get(canister_check);
    if (consumable.type != ENTITY_TYPE::OXYGEN_CANISTER ||
        canister_check == hit_entity || canister_check == proj ||
        !registry.positions.has(hit_entity)) {
      continue;
    }

    // ignore if explosion did not touch canister
    Position& canister_position = registry.positions.get(canister_check);
    if (circle_box_collides(playerproj_position, playerproj_aoe.radius,
                            canister_position)) {
      registry.consumables.remove(canister_check);
      resolveCanisterPlayerProjCollision(canister_check, proj);
    }
  }
}

void CollisionSystem::detectAndResolveConeAOE(Entity proj, Entity enemy,
                                              float angle) {
  for (Entity enemy_check : registry.deadlys.entities) {
    if (enemy_check == enemy || !registry.positions.has(enemy)) {
      continue;
    }
    Position&     playerproj_position = registry.positions.get(proj);
    AreaOfEffect& playerproj_aoe      = registry.aoe.get(proj);
    Position&     enemy_position      = registry.positions.get(enemy_check);

    float circle_angle = playerproj_position.angle;
    vec2  pos_diff     = playerproj_position.position - enemy_position.position;
    float proj_ent_angle = atan2(pos_diff.y, pos_diff.x);
    if (registry.playerProjectiles.get(proj).is_flipped) {
      circle_angle -= M_PI;
    }
    if (circle_angle < 0) {
      circle_angle += 2 * M_PI;
    }
    if (proj_ent_angle < 0) {
      proj_ent_angle += 2 * M_PI;
    }

    float anglediff =
        fmod((circle_angle - proj_ent_angle + 3 * M_PI), 2 * M_PI);

    float is_inbetween = abs(anglediff) <= angle;

    /*proj_ent_angle = std::fmod(2.f * M_PI + proj_ent_angle, 2.f * M_PI);
    float min      = std::fmod(20.f * M_PI + circle_angle - angle, 2.f * M_PI);
    float max      = std::fmod(20.f * M_PI + circle_angle + angle, 2.f * M_PI);

    bool is_inbetween = !(min <= proj_ent_angle) || (proj_ent_angle <= max);

    if (min < max) {
      is_inbetween = !(min <= proj_ent_angle) && (proj_ent_angle <= max);
    }*/

    /*if (circle_box_collides(playerproj_position, playerproj_aoe.radius,
                            enemy_position)) {
      printf("Proj angle %f\n Distance Angle %f\n", anglediff, angle);
    }*/

    if (circle_box_collides(playerproj_position, playerproj_aoe.radius,
                            enemy_position) &&
        is_inbetween) {
      modifyOxygen(enemy_check, proj);
      addDamageIndicatorTimer(enemy_check);
    }
  }
}

void CollisionSystem::resolveWallPlayerProjCollision(Entity wall,
                                                     Entity player_proj) {
  if (!registry.motions.has(player_proj) ||
      !registry.playerProjectiles.has(player_proj)) {
    return;
  }
  Motion&           proj_motion = registry.motions.get(player_proj);
  PlayerProjectile& proj_component =
      registry.playerProjectiles.get(player_proj);
  Inventory& inventory = registry.inventory.get(player);

  bool check_wep_swap      = player_projectile != player_proj;
  proj_motion.velocity     = vec2(0.f);
  proj_component.is_loaded = true;
  if (proj_component.type == PROJECTILES::SHRIMP) {
    if (check_wep_swap) {
      destroyGunOrProjectile(player_proj);
    }
  } else if (proj_component.type == PROJECTILES::CONCUSSIVE) {
    if (check_wep_swap) {
      destroyGunOrProjectile(player_proj);
    }
    if (inventory.concussors <= 0) {
      doWeaponSwap(harpoon, harpoon_gun, PROJECTILES::HARPOON);
      changeSelectedCounterColour(INVENTORY::HARPOON);
    }
  } else {
    if (proj_component.type == PROJECTILES::TORPEDO) {
      makeTorpedoExplosion(renderer,
                           registry.positions.get(player_proj).position);
      detectAndResolveExplosion(player_proj, wall);
    }
    proj_motion.velocity     = vec2(0.f);
    proj_component.is_loaded = true;
  }
}

void CollisionSystem::resolveWallEnemyProjCollision(Entity wall,
                                                    Entity enemy_proj) {
  // canister projectiles explode on walls
  if (registry.enemyProjectiles.has(enemy_proj) &&
      registry.enemyProjectiles.get(enemy_proj).type ==
          ENTITY_TYPE::OXYGEN_CANISTER) {
    detectAndResolveExplosion(enemy_proj, wall);
    make_canister_explosion(renderer,
                            registry.positions.get(enemy_proj).position);
  }
  registry.remove_all_components_of(enemy_proj);
}

void CollisionSystem::resolveMassInteractableCollision(Entity mass,
                                                       Entity interactable) {
  if (!registry.masses.has(mass)) {
    return;
  }

  Mass& mass_comp = registry.masses.get(mass);
  if (registry.pressurePlates.has(interactable)) {
    PressurePlate& pp = registry.pressurePlates.get(interactable);
    if (!pp.active && pp.mass_activation <= mass_comp.mass) {
      registry.renderRequests.get(interactable).used_texture =
          TEXTURE_ASSET_ID::PRESSURE_PLATE_ON;
      pp.active = true;
      if (!registry.sounds.has(interactable)) {
        registry.sounds.insert(interactable,
                               Sound(SOUND_ASSET_ID::PRESSURE_PLATE));
      }

      // Connect to an available door, if we haven't yet.
      // Exploits the fact that there's only 1 PP per room.
      for (Entity& entity : registry.activeDoors.entities) {
        if (registry.doorConnections.has(entity)) {
          DoorConnection& door_connection = registry.doorConnections.get(entity);
          if (door_connection.objective == Objective::PRESSURE_PLATE) {
            door_connection.locked = false;

            // change the sprite
            if (registry.renderRequests.has(entity)) {
              registry.renderRequests.remove(entity);

              level->assign_door_sprite(entity, door_connection);
            }
          }
        }
      }
    }
  }
}

void CollisionSystem::resolveWallEnemyCollision(Entity wall, Entity enemy) {
  if (!registry.motions.has(enemy) || !registry.positions.has(enemy)) {
    return;
  }

  Motion&   enemy_motion   = registry.motions.get(enemy);
  Position& enemy_position = registry.positions.get(enemy);
  Position& wall_position  = registry.positions.get(wall);
  vec2 wall_dir = normalize(wall_position.position - enemy_position.position);
  vec2 temp_velocity = enemy_motion.velocity;

  resolveStopOnWall(wall, enemy);

  // if the enemy is actively tracking the player, route them around the wall
  if ((registry.trackPlayer.has(enemy) &&
       registry.trackPlayer.get(enemy).active_track) ||
      (registry.trackPlayer.has(enemy) &&
       registry.trackPlayer.get(enemy).active_track)) {
    vec2  enemy_dir = normalize(temp_velocity);
    float velocity  = sqrt(dot(temp_velocity, temp_velocity));
    float acceleration =
        sqrt(dot(enemy_motion.acceleration, enemy_motion.acceleration));
    vec2 new_dir              = normalize(enemy_dir - wall_dir);
    enemy_motion.velocity     = new_dir * velocity;
    enemy_motion.acceleration = new_dir * acceleration;
  } else {
    enemy_motion.velocity = temp_velocity;
    // adjust enemy ai
    enemy_motion.velocity *= -1.0f;
    enemy_motion.acceleration *= -1.0f;
  }
  enemy_position.scale.x = abs(enemy_position.scale.x);
  if (enemy_motion.velocity.x > 0) {
    enemy_position.scale.x *= -1.0f;
  }

  if (registry.bosses.has(enemy)) {
    Boss& boss = registry.bosses.get(enemy);
    if (boss.type == ENTITY_TYPE::SHARKMAN) {
      // break crates if sharkman hits them while targeting player
      if (registry.breakables.has(wall) && registry.trackPlayer.has(enemy) &&
          is_tracking(enemy)) {
        modifyOxygenAmount(enemy, SHARKMAN_SELF_DMG);
        modifyOxygenAmount(wall, SHARKMAN_SELF_DMG);
        if (!registry.sounds.has(wall)) {
          registry.sounds.insert(wall,
                        Sound(SOUND_ASSET_ID::METAL_CRATE_DEATH));
        }
        Motion& motion = registry.motions.get(enemy);
        float   speed  = sqrt(dot(motion.velocity, motion.velocity));
        motion.velocity =
            normalize(motion.velocity) * (speed + (float)SHARKMAN_MS_INC);

        // reset sharkman to wander
        printf("Sharkman hit a crate, resetting to wander\n");
        boss.curr_cd = SHARKMAN_AI_CD;
        removeFromAI(enemy);
        addSharkmanWander();
      }
      choose_new_direction(enemy, wall);
    }
  }
}

void CollisionSystem::resolveStopOnWall(Entity wall, Entity entity) {
  Position& wall_position   = registry.positions.get(wall);
  Position& entity_position = registry.positions.get(entity);

  vec4 wall_bounds = get_bounds(wall_position);

  float left_bound_wall  = wall_bounds[0];
  float right_bound_wall = wall_bounds[1];
  float top_bound_wall   = wall_bounds[2];
  float bot_bound_wall   = wall_bounds[3];

  vec4 entity_bounds = get_bounds(entity_position);

  float left_bound_entity  = entity_bounds[0];
  float right_bound_entity = entity_bounds[1];
  float top_bound_entity   = entity_bounds[2];
  float bot_bound_entity   = entity_bounds[3];

  float entitys_right_overlaps_left_wall =
      (right_bound_entity - left_bound_wall);
  float entitys_left_overlaps_right_wall =
      (right_bound_wall - left_bound_entity);
  float entitys_bot_overlaps_top_wall = (bot_bound_entity - top_bound_wall);
  float entitys_top_overlaps_bot_wall = (bot_bound_wall - top_bound_entity);

  // We need to find the smallest overlap horizontally and verticallly to
  // determine where the overlap happened.
  float overlapX =
      min(entitys_right_overlaps_left_wall, entitys_left_overlaps_right_wall);
  float overlapY =
      min(entitys_bot_overlaps_top_wall, entitys_top_overlaps_bot_wall);

  bool  isLeftOfWall = (entity_position.position.x < wall_position.position.x);
  bool  isAboveWall  = (entity_position.position.y < wall_position.position.y);
  float overlapThreshold       = 0.1f;
  float overlapPushbackPercent = 0.5f;

  // If the overlap in the X direction is smaller, it means that the collision
  // occured here. Vice versa for overlap in Y direction.
  if (overlapX < overlapY) {
    // If the entity is on the left of the wall, we push left.
    // If the entity is on the right of the wall, we push right.
    overlapX = isLeftOfWall ? -1 * overlapX : overlapX;

    // The smallest overlap is a large value means we got stuck
    // between crate/wall. Resolve by pushing away crate.
    if (abs(overlapX) > overlapThreshold) {
      if (registry.breakables.has(wall) && registry.motions.has(wall)) {
        Motion& crate_pos = registry.motions.get(wall);
        crate_pos.velocity.x -= overlapX * overlapPushbackPercent;
      }
    }

    entity_position.position.x += overlapX;

    if (registry.players.has(entity)) {
      Player player_comp = registry.players.get(entity);
      if (registry.positions.has(player_comp.collisionMesh)) {
        registry.positions.get(player_comp.collisionMesh).position.x +=
            overlapX;
      }
    }

    if (registry.motions.has(entity) && !registry.players.has(entity)) {
      registry.motions.get(entity).velocity.x = 0;
    }
  } else {
    // If the entity is above the wall, then we need to push up.
    // If the entity is below the wall, then we need to push down.
    overlapY = isAboveWall ? -1 * overlapY : overlapY;

    // The smallest overlap is a large value means we got stuck
    // between crate/wall. Resolve by pushing away crate.
    if (abs(overlapY) > overlapThreshold) {
      if (registry.breakables.has(wall) && registry.motions.has(wall)) {
        Motion& crate_pos = registry.motions.get(wall);
        crate_pos.velocity.y -= overlapY * overlapPushbackPercent;
      }
    }

    entity_position.position.y += overlapY;

    if (registry.players.has(entity)) {
      Player player_comp = registry.players.get(entity);
      if (registry.positions.has(player_comp.collisionMesh)) {
        registry.positions.get(player_comp.collisionMesh).position.y +=
            overlapY;
      }
    }

    if (registry.motions.has(entity) && !registry.players.has(entity)) {
      registry.motions.get(entity).velocity.y = 0;
    }
  }
}

void CollisionSystem::resolveMassCollision(Entity wall, Entity other) {
  Motion&   wall_motion  = registry.motions.get(wall);
  Motion&   other_motion = registry.motions.get(other);
  Position& wall_pos     = registry.positions.get(wall);
  Position& other_pos    = registry.positions.get(other);

  bool is_horizontal_collision = false;
  // Determine if this a horizontal or vertical collision
  vec2 pos_diff = wall_pos.position - other_pos.position;

  if (abs(pos_diff.x) > abs(pos_diff.y)) {
    is_horizontal_collision = true;
  }

  // This is here because sometimes it still counts as a collision, even if they
  // aren't colliding
  float wall_position =
      is_horizontal_collision ? wall_pos.position.x : wall_pos.position.y;
  float wall_velo =
      is_horizontal_collision ? wall_motion.velocity.x : wall_motion.velocity.y;
  float other_position =
      is_horizontal_collision ? other_pos.position.x : other_pos.position.y;
  float other_velo = is_horizontal_collision ? other_motion.velocity.x
                                             : other_motion.velocity.y;
  if ((other_position > wall_position && other_velo > wall_velo) ||
      (other_position < wall_position && other_velo < wall_velo)) {
    return;
  }

  // Assume these collisions are inelastic - that is, that kinetic energy is
  // preserved

  float wall_mass  = registry.masses.get(wall).mass;
  float other_mass = registry.masses.get(other).mass;
  // p = momentum
  float p_wall  = is_horizontal_collision ? wall_mass * wall_motion.velocity.x
                                          : wall_mass * wall_motion.velocity.y;
  float p_other = is_horizontal_collision
                      ? other_mass * other_motion.velocity.x
                      : other_mass * other_motion.velocity.y;

  float v_final = (p_wall + p_other) / (wall_mass + other_mass);

  if (is_horizontal_collision) {
    wall_motion.velocity.x  = v_final;
    other_motion.velocity.x = v_final;
  } else {
    wall_motion.velocity.y  = v_final;
    other_motion.velocity.y = v_final;
  }
}

void CollisionSystem::resolveDoorPlayerCollision(Entity door, Entity player) {
  if (registry.doorConnections.has(door)) {
    DoorConnection& doorConnection = registry.doorConnections.get(door);
    DoorConnection& otherDoorConnection =
        registry.doorConnections.get(doorConnection.exit_door);
    bool is_boss_room = otherDoorConnection.room_id == "5" ||
                        otherDoorConnection.room_id == "10" ||
                        otherDoorConnection.room_id == "15";

    // If the door is locked, ignore it.
    if (doorConnection.locked && is_boss_room) {
      bossLockedDialogue(renderer);
      return;
    } else if (doorConnection.locked && otherDoorConnection.room_id == "0") {
      tutorialLockedDialogue(renderer);
      return;
    } else if (doorConnection.locked && (doorConnection.objective == Objective::RED_KEY || doorConnection.objective == Objective::BLUE_KEY || doorConnection.objective == Objective::YELLOW_KEY)) {
      keyLockedDialogue(renderer);
      return;
    } else if (doorConnection.locked && doorConnection.objective == Objective::PRESSURE_PLATE) {
      // No clue why, but this breaks if I change it to plateLockedDialogue, so I just changed keyLockedDialogue, LOL.
      keyLockedDialogue(renderer);
      return;
    } else if (doorConnection.locked && doorConnection.objective == Objective::PRESSURE_PLATE) {
      plateLockedDialogue(renderer);
      return;
    }
  }

  DoorConnection& door_connection = registry.doorConnections.get(door);
  rt_entity                       = Entity();
  RoomTransition& roomTransition  = registry.roomTransitions.emplace(rt_entity);
  roomTransition.door_connection  = door_connection;

  room_transitioning = true;

  registry.sounds.insert(rt_entity, Sound(SOUND_ASSET_ID::DOOR));

  PlayerProjectile& pp   = registry.playerProjectiles.get(player_projectile);
  Motion&           pp_m = registry.motions.get(player_projectile);
  pp.is_loaded           = true;
  pp_m.velocity          = {0.f, 0.f};
}
