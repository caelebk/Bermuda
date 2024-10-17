#include "collision_system.hpp"

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

// Collision Detection System
void CollisionSystem::collision_detection() {
  ComponentContainer<Position>& position_container = registry.positions;
  for (uint i = 0; i < position_container.components.size(); i++) {
    Position& position_i = position_container.components[i];
    Entity    entity_i   = position_container.entities[i];

    for (uint j = i + 1; j < position_container.components.size(); j++) {
      Position& position_j = position_container.components[j];
      Entity    entity_j   = position_container.entities[j];

      /********** Collisions to Ignore *************/

      // no collision component
      if (!registry.collidables.has(entity_i) ||
          !registry.collidables.has(entity_j)) {
        continue;
      }

      // they don't collide
      if (!box_collides(position_i, position_j)) {
        continue;
      }

      // wall to wall
      if (registry.activeWalls.has(entity_i) &&
          registry.activeWalls.has(entity_j)) {
        continue;
      }

      // wall <-> interactable
      if (registry.activeWalls.has(entity_i) &&
          registry.interactable.has(entity_j)) {
        continue;
      }
      if (registry.activeWalls.has(entity_j) &&
          registry.interactable.has(entity_i)) {
        continue;
      }

      // wall <-> consumable
      if (registry.activeWalls.has(entity_i) &&
          registry.consumables.has(entity_j)) {
        continue;
      }
      if (registry.activeWalls.has(entity_j) &&
          registry.consumables.has(entity_i)) {
        continue;
      }

      // interactable <-> consumable
      if (registry.interactable.has(entity_i) &&
          registry.consumables.has(entity_j)) {
        continue;
      }
      if (registry.interactable.has(entity_j) &&
          registry.consumables.has(entity_i)) {
        continue;
      }

      // interactable <-> interactable
      if (registry.interactable.has(entity_j) &&
          registry.consumables.has(entity_i)) {
        continue;
      }

      // consumable <-> consumable
      if (registry.interactable.has(entity_j) &&
          registry.consumables.has(entity_i)) {
        continue;
      }

      // enemy <-> consumable
      if (registry.deadlys.has(entity_i) &&
          registry.consumables.has(entity_j)) {
        continue;
      }
      if (registry.deadlys.has(entity_j) &&
          registry.consumables.has(entity_i)) {
        continue;
      }

      // enemy <-> interactable
      if (registry.deadlys.has(entity_i) &&
          registry.interactable.has(entity_j)) {
        continue;
      }
      if (registry.deadlys.has(entity_j) &&
          registry.interactable.has(entity_i)) {
        continue;
      }

      // enemy to enemy
      if (registry.deadlys.has(entity_i) && registry.deadlys.has(entity_j)) {
        continue;
      }

      // player <-> player projectile
      bool player_to_playerproj = registry.players.has(entity_i) &&
                                  registry.playerProjectiles.has(entity_j);
      bool playerproj_to_player = registry.playerProjectiles.has(entity_i) &&
                                  registry.players.has(entity_j);

      if (player_to_playerproj || playerproj_to_player) {
        continue;
      }

      // loaded harpoon should not collide
      bool loaded_playerproj_i =
          registry.playerProjectiles.has(entity_i) &&
          registry.playerProjectiles.get(entity_i).is_loaded;
      bool loaded_playerproj_j =
          registry.playerProjectiles.has(entity_j) &&
          registry.playerProjectiles.get(entity_j).is_loaded;

      if (loaded_playerproj_i || loaded_playerproj_j) {
        continue;
      }

      // None of the above, then proper collisions were detected.
      registry.collisions.emplace_with_duplicates(entity_i, entity_j);
      registry.collisions.emplace_with_duplicates(entity_j, entity_i);
    }
  }
}

void CollisionSystem::collision_resolution_debug_info(Entity entity,
                                                      Entity entity_other) {
  printf("Entity:\n");
  registry.list_all_components_of(entity);
  printf("Entity Other:\n");
  registry.list_all_components_of(entity_other);

  if (registry.players.has(entity)) {
    printf("Player -");
    if (registry.activeWalls.has(entity_other)) {
      printf("Wall\n");
    } else if (registry.consumables.has(entity_other)) {
      printf("Eat\n");
    } else if (registry.deadlys.has(entity_other)) {
      printf("Deadly\n");
    } else if (registry.players.has(entity_other)) {
      printf("Player\n");
    } else if (registry.playerWeapons.has(entity_other)) {
      printf("Player Weap\n");
    } else if (registry.oxygen.has(entity_other)) {
      printf("Oxygen\n");
    } else if (registry.playerProjectiles.has(entity_other)) {
      printf("Player Projectile\n");
    }
  }
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

    // Enemy Collision Handling
    if (registry.deadlys.has(entity)) {
      routeEnemyCollisions(entity, entity_other);
    }

    // Wall Collision Handling
    if (registry.activeWalls.has(entity)) {
      routeWallCollisions(entity, entity_other);
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
    resolveWallPlayerCollision(other, player);
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
    resolveWallPlayerCollision(wall, other);
  }
  if (registry.playerProjectiles.has(other)) {
    resolveWallPlayerProjCollision(wall, other);
  }
  if (registry.deadlys.has(other)) {
    resolveWallEnemyCollision(wall, other);
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
  Motion&   enemy_motion   = registry.motions.get(enemy);
  Position& enemy_position = registry.positions.get(enemy);

  enemy_motion.velocity *= -1.0f;
  enemy_position.scale.x *= -1.0f;
}

void CollisionSystem::resolveWallPlayerCollision(Entity wall, Entity player) {
  Motion& player_motion  = registry.motions.get(player);
  player_motion.velocity = vec2(0.0f, 0.0f);

  Position& wall_position   = registry.positions.get(wall);
  Position& player_position = registry.positions.get(player);

  vec4 wall_bounds = get_bounds(wall_position);

  float left_bound_wall  = wall_bounds[0];
  float right_bound_wall = wall_bounds[1];
  float top_bound_wall   = wall_bounds[2];
  float bot_bound_wall   = wall_bounds[3];

  vec4 player_bounds = get_bounds(player_position);

  float left_bound_player  = player_bounds[0];
  float right_bound_player = player_bounds[1];
  float top_bound_player   = player_bounds[2];
  float bot_bound_player   = player_bounds[3];

  float players_right_overlaps_left_wall =
      (right_bound_player - left_bound_wall);
  float players_left_overlaps_right_wall =
      (right_bound_wall - left_bound_player);
  float players_bot_overlaps_top_wall = (bot_bound_player - top_bound_wall);
  float players_top_overlaps_bot_wall = (bot_bound_wall - top_bound_player);

  // We need to find the smallest overlap horizontally and verticallly to
  // determine where the overlap happened.
  float overlapX =
      min(players_right_overlaps_left_wall, players_left_overlaps_right_wall);
  float overlapY =
      min(players_bot_overlaps_top_wall, players_top_overlaps_bot_wall);

  // If the overlap in the X direction is smaller, it means that the collision
  // occured here. Vice versa for overlap in Y direction.
  if (overlapX < overlapY) {
    // Respective to the wall,
    // If the player is on the left of the wall, then we need to push him left.
    // If the player is on the right of the wall, we need to push him right.
    overlapX = (player_position.position.x < wall_position.position.x)
                   ? -1 * overlapX
                   : overlapX;
    player_position.position.x += overlapX;
  } else {
    // Respective to the wall,
    // If the player is above the wall, then we need to push up left.
    // If the player is below the wall, then we need to push him down.
    overlapY = (player_position.position.y < wall_position.position.y)
                   ? -1 * overlapY
                   : overlapY;
    player_position.position.y += overlapY;
  }
}