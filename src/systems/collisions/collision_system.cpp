#include "collision_system.hpp"
#include <cstdio>
#include <iostream>

#include "ai.hpp"
#include "enemy.hpp"
#include "oxygen.hpp"
#include "tiny_ecs_registry.hpp"

// Returns the local bounding coordinates scaled by entity size
vec2 get_bounding_box(const Position& position) {
  return {abs(position.scale.x), abs(position.scale.y)};
}

// Returns the rectangular bounds
vec4 get_bounds(const Position& position) {
  vec2  bounding_box        = get_bounding_box(position);
  float horizontal_dist_box = (bounding_box.x) / 2.0f;
  float vertical_dist_box   = (bounding_box.y) / 2.0f;

  float left_bound  = position.position.x - horizontal_dist_box;
  float right_bound = position.position.x + horizontal_dist_box;
  float top_bound   = position.position.y - vertical_dist_box;
  float bot_bound   = position.position.y + vertical_dist_box;

  return vec4(left_bound, right_bound, top_bound, bot_bound);
}

bool circle_collides(const Position& position1, const Position& position2) {
  vec2        dp                = position1.position - position2.position;
  float       dist_squared      = dot(dp, dp);
  const vec2  other_bonding_box = get_bounding_box(position1) / 2.f;
  const float other_r_squared   = dot(other_bonding_box, other_bonding_box);
  const vec2  my_bonding_box    = get_bounding_box(position2) / 2.f;
  const float my_r_squared      = dot(my_bonding_box, my_bonding_box);
  const float r_squared         = max(other_r_squared, my_r_squared);
  if (dist_squared < r_squared) return true;
  return false;
}

// Axis-Aligned Bounding Box Collision detection.
bool box_collides(const Position& position1, const Position& position2) {
  vec4 box1_bounds = get_bounds(position1);

  float left_bound_box1  = box1_bounds[0];
  float right_bound_box1 = box1_bounds[1];
  float top_bound_box1   = box1_bounds[2];
  float bot_bound_box1   = box1_bounds[3];

  vec4 box2_bounds = get_bounds(position2);

  float left_bound_box2  = box2_bounds[0];
  float right_bound_box2 = box2_bounds[1];
  float top_bound_box2   = box2_bounds[2];
  float bot_bound_box2   = box2_bounds[3];

  bool vertical_overlap    = top_bound_box1 < bot_bound_box2;
  bool vertical_overlap2   = top_bound_box2 < bot_bound_box1;
  bool horizontal_overlap  = left_bound_box1 < right_bound_box2;
  bool horizontal_overlap2 = left_bound_box2 < right_bound_box1;

  return vertical_overlap && vertical_overlap2 && horizontal_overlap &&
         horizontal_overlap2;
}

void CollisionSystem::step(float elapsed_ms) {
  collision_detection();
  collision_resolution();
}

/***********************************
Collision Detection (has precedence noted below)
************************************/
void CollisionSystem::collision_detection() {
  ComponentContainer<Player>&           player_container = registry.players;
  ComponentContainer<PlayerProjectile>& playerproj_container =
      registry.playerProjectiles;
  ComponentContainer<Deadly>&       enemy_container      = registry.deadlys;
  ComponentContainer<ActiveWall>&   wall_container       = registry.activeWalls;
  ComponentContainer<ActiveDoor>&   door_container       = registry.activeDoors;
  ComponentContainer<Consumable>&   consumable_container = registry.consumables;
  ComponentContainer<Interactable>& interactable_container =
      registry.interactable;

  // 1. Detect player projectile collisions
  for (uint i = 0; i < playerproj_container.components.size(); i++) {
    Entity entity_i = playerproj_container.entities[i];
    if (!registry.positions.has(entity_i)) {
      continue;
    }
    Position& position_i = registry.positions.get(entity_i);

    if (registry.playerProjectiles.get(entity_i).is_loaded) {
      continue;
    }

    for (uint j = 0; j < enemy_container.size(); j++) {
      Entity entity_j = enemy_container.entities[j];
      if (!registry.positions.has(entity_j)) {
        continue;
      }

      Position& position_j = registry.positions.get(entity_j);
      if (circle_collides(position_i, position_j)) {
        registry.collisions.emplace_with_duplicates(entity_i, entity_j);
        registry.collisions.emplace_with_duplicates(entity_j, entity_i);
      }
    }

    for (uint j = 0; j < wall_container.size(); j++) {
      Entity entity_j = wall_container.entities[j];
      if (!registry.positions.has(entity_j)) {
        continue;
      }
      Position& position_j = registry.positions.get(entity_j);
      if (box_collides(position_i, position_j)) {
        registry.collisions.emplace_with_duplicates(entity_i, entity_j);
        registry.collisions.emplace_with_duplicates(entity_j, entity_i);
      }
    }
  }

  // 2. Detect player collisions
  for (uint i = 0; i < player_container.components.size(); i++) {
    Entity entity_i = player_container.entities[i];
    if (!registry.positions.has(entity_i)) {
      continue;
    }
    Position& position_i = registry.positions.get(entity_i);

    for (uint j = 0; j < enemy_container.size(); j++) {
      Entity entity_j = enemy_container.entities[j];
      if (!registry.positions.has(entity_j)) {
        continue;
      }

      // don't detect the enemy collision if their attack is on cooldown
      if (registry.modifyOxygenCd.has(entity_j)) {
        ModifyOxygenCD& modifyOxygenCd = registry.modifyOxygenCd.get(entity_j);
        if (modifyOxygenCd.curr_cd > 0.f) {
          continue;
        }
      }
      Position& position_j = registry.positions.get(entity_j);
      if (box_collides(position_i, position_j)) {
        registry.collisions.emplace_with_duplicates(entity_i, entity_j);
        registry.collisions.emplace_with_duplicates(entity_j, entity_i);
      }
    }

    for (uint j = 0; j < consumable_container.size(); j++) {
      Entity entity_j = consumable_container.entities[j];
      if (!registry.positions.has(entity_j)) {
        continue;
      }
      Position& position_j = registry.positions.get(entity_j);
      if (box_collides(position_i, position_j)) {
        registry.collisions.emplace_with_duplicates(entity_i, entity_j);
        registry.collisions.emplace_with_duplicates(entity_j, entity_i);
      }
    }

    for (uint j = 0; j < interactable_container.size(); j++) {
      Entity entity_j = interactable_container.entities[j];
      if (!registry.positions.has(entity_j)) {
        continue;
      }

      // don't detect the interactable collision if their attack is on cooldown
      if (registry.modifyOxygenCd.has(entity_j)) {
        ModifyOxygenCD& modifyOxygenCd = registry.modifyOxygenCd.get(entity_j);
        if (modifyOxygenCd.curr_cd > 0.f) {
          continue;
        }
      }

      Position& position_j = registry.positions.get(entity_j);
      if (box_collides(position_i, position_j)) {
        registry.collisions.emplace_with_duplicates(entity_i, entity_j);
        registry.collisions.emplace_with_duplicates(entity_j, entity_i);
      }
    }
  }

  // 3. Detect wall collisions
  for (uint i = 0; i < wall_container.components.size(); i++) {
    Entity entity_i = wall_container.entities[i];
    if (!registry.positions.has(entity_i)) {
      continue;
    }
    Position& position_i = registry.positions.get(entity_i);

    for (uint j = 0; j < enemy_container.size(); j++) {
      Entity entity_j = enemy_container.entities[j];
      if (!registry.positions.has(entity_j)) {
        continue;
      }

      if (entity_i == entity_j) {
        // TODO: this is a bit of a hack, crates are enemies that are walls
        continue;
      }
      Position& position_j = registry.positions.get(entity_j);
      if (box_collides(position_i, position_j)) {
        registry.collisions.emplace_with_duplicates(entity_i, entity_j);
        registry.collisions.emplace_with_duplicates(entity_j, entity_i);
      }
    }

    for (uint j = 0; j < player_container.size(); j++) {
      Entity entity_j = player_container.entities[j];
      if (!registry.positions.has(entity_j)) {
        continue;
      }
      Position& position_j = registry.positions.get(entity_j);
      if (box_collides(position_i, position_j)) {
        registry.collisions.emplace_with_duplicates(entity_i, entity_j);
        registry.collisions.emplace_with_duplicates(entity_j, entity_i);
      }
    }
  }

  // 4. Detect door collisions
  for (uint i = 0; i < door_container.components.size(); i++) {
    Entity entity_i = door_container.entities[i];
    if (!registry.positions.has(entity_i)) {
      continue;
    }
    Position& position_i = registry.positions.get(entity_i);

    for (uint j = 0; j < enemy_container.size(); j++) {
      Entity entity_j = enemy_container.entities[j];
      if (!registry.positions.has(entity_j)) {
        continue;
      }

      if (entity_i == entity_j) {
        // TODO: this is a bit of a hack, crates are enemies that are walls
        continue;
      }
      Position& position_j = registry.positions.get(entity_j);
      if (box_collides(position_i, position_j)) {
        registry.collisions.emplace_with_duplicates(entity_i, entity_j);
        registry.collisions.emplace_with_duplicates(entity_j, entity_i);
      }
    }

    for (uint j = 0; j < player_container.size(); j++) {
      Entity entity_j = player_container.entities[j];
      if (!registry.positions.has(entity_j)) {
        continue;
      }
      Position& position_j = registry.positions.get(entity_j);
      if (box_collides(position_i, position_j)) {
        registry.collisions.emplace_with_duplicates(entity_i, entity_j);
        registry.collisions.emplace_with_duplicates(entity_j, entity_i);
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

    // Consumable Collision Handling
    if (registry.consumables.has(entity)) {
      routeConsumableCollisions(entity, entity_other);
    }

    // Interactable Collision Handling
    if (registry.interactable.has(entity)) {
      routeInteractableCollisions(entity, entity_other);
    }

    // Remove all collisions from this simulation step
    registry.collisions.clear();
  }
}

/*********************************************
  Entity -> Other Entity Collision Routing
**********************************************/
void CollisionSystem::routePlayerCollisions(Entity player, Entity other) {
  if (registry.deadlys.has(other)) {
    resolvePlayerEnemyCollision(player, other);
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
  if (registry.players.has(other)) {
    resolvePlayerEnemyCollision(other, enemy);
  }
  if (registry.playerProjectiles.has(other)) {
    resolveEnemyPlayerProjCollision(enemy, other);
  }
  if (registry.activeWalls.has(other)) {
    resolveWallEnemyCollision(other, enemy);
  }
}

void CollisionSystem::routeWallCollisions(Entity wall, Entity other) {
  if (!registry.motions.has(other)) {
    return;
  }

  if (registry.players.has(other)) {
    resolveStopOnWall(wall, other);
  }
  if (registry.playerProjectiles.has(other)) {
    resolveWallPlayerProjCollision(wall, other);
  }
  if (registry.deadlys.has(other)) {
    resolveWallEnemyCollision(wall, other);
  }
}

void CollisionSystem::routeDoorCollisions(Entity door, Entity other) {
  if (!registry.motions.has(other)) {
    return;
  }

  if (registry.players.has(other)) {
    resolveDoorPlayerCollision(door, other);
  }

  // Since enemies and projectiles can't enter different rooms, simply treat their collisions like a wall.
  if (registry.playerProjectiles.has(other)) {
    resolveWallPlayerProjCollision(door, other);
  }
  if (registry.deadlys.has(other)) {
    resolveWallEnemyCollision(door, other);
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
  if (player_proj != player_projectile) {
    registry.motions.remove(player_proj);
    registry.positions.remove(player_proj);
    registry.renderRequests.remove(player_proj);
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
}

/*********************************************
    Entity <-> Entity Collision Resolutions
**********************************************/
void CollisionSystem::resolvePlayerEnemyCollision(Entity player, Entity enemy) {
  handle_debuffs(player, enemy);
  modifyOxygen(player, enemy);
}

void CollisionSystem::resolvePlayerConsumableCollision(Entity player,
                                                       Entity consumable) {
  if (registry.deathTimers.has(player)) {
    return;
  }

  // TODO: add more affects M2+

  // will add oxygen to the player if it exists
  modifyOxygen(player, consumable);
  registry.remove_all_components_of(consumable);
}

void CollisionSystem::resolvePlayerInteractableCollision(Entity player,
                                                         Entity interactable) {
  if (registry.deathTimers.has(player)) {
    return;
  }
  // TODO: add more affects M2+

  // will add oxygen to the player if it exists
  modifyOxygen(player, interactable);
}

void CollisionSystem::resolveEnemyPlayerProjCollision(Entity enemy,
                                                      Entity player_proj) {
  PlayerProjectile& player_projectile =
      registry.playerProjectiles.get(player_proj);
  Motion& playerproj_motion = registry.motions.get(player_proj);

  modifyOxygen(enemy, player_proj);
  playerproj_motion.velocity  = vec2(0.0f, 0.0f);
  player_projectile.is_loaded = true;

  handle_debuffs(enemy, player_proj);
  
  // make enemies that track the player briefly start tracking them regardless of range
  if (registry.trackPlayer.has(enemy)) {
    TracksPlayer &tracks = registry.trackPlayer.get(enemy);
    tracks.active_track = true;
  }
}

void CollisionSystem::resolveWallPlayerProjCollision(Entity wall,
                                                     Entity player_proj) {
  Motion&           proj_motion = registry.motions.get(player_proj);
  PlayerProjectile& player_projectile =
      registry.playerProjectiles.get(player_proj);

  proj_motion.velocity        = vec2(0.0, 0.0);
  player_projectile.is_loaded = true;
}

void CollisionSystem::resolveWallEnemyCollision(Entity wall, Entity enemy) {
  if (!registry.motions.has(enemy) || !registry.positions.has(enemy)) {
    return;
  }
  Motion&   enemy_motion   = registry.motions.get(enemy);
  Position& enemy_position = registry.positions.get(enemy);
  vec2      temp_velocity  = enemy_motion.velocity;

  resolveStopOnWall(wall, enemy);

  enemy_motion.velocity = temp_velocity;

  // adjust enemy ai
  enemy_motion.velocity *= -1.0f;

  enemy_position.scale.x = abs(enemy_position.scale.x);
  if (enemy_motion.velocity.x > 0) {
    enemy_position.scale.x *= -1.0f;
  }
}

void CollisionSystem::resolveStopOnWall(Entity wall, Entity entity) {
  Motion& entity_motion  = registry.motions.get(entity);
  entity_motion.velocity = vec2(0.0f, 0.0f);

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

  // If the overlap in the X direction is smaller, it means that the collision
  // occured here. Vice versa for overlap in Y direction.
  if (overlapX < overlapY) {
    // Respective to the wall,
    // If the entity is on the left of the wall, then we need to push him left.
    // If the entity is on the right of the wall, we need to push him right.
    overlapX = (entity_position.position.x < wall_position.position.x)
                   ? -1 * overlapX
                   : overlapX;
    entity_position.position.x += overlapX;
  } else {
    // Respective to the wall,
    // If the entity is above the wall, then we need to push up left.
    // If the entity is below the wall, then we need to push him down.
    overlapY = (entity_position.position.y < wall_position.position.y)
                   ? -1 * overlapY
                   : overlapY;
    entity_position.position.y += overlapY;
  }
}

void CollisionSystem::resolveDoorPlayerCollision(Entity door, Entity player) {
  DoorConnection& door_connection = registry.doorConnections.get(door);
  level_builder.switch_room(door_connection);
}
