// internal
#include "physics_system.hpp"
#include "audio_system.hpp"
#include "consumable_utils.hpp"
#include "debuff.hpp"
#include "map_util.hpp"
#include "oxygen_system.hpp"
#include "physics.hpp"
#include "player_factories.hpp"
#include "tiny_ecs_registry.hpp"

// Returns the local bounding coordinates scaled by entity size
vec2 get_bounding_box(const Position &position) {
  return {abs(position.scale.x), abs(position.scale.y)};
}

bool circle_collides(const Position &position1, const Position &position2) {
  vec2 dp = position1.position - position2.position;
  float dist_squared = dot(dp, dp);
  const vec2 other_bonding_box = get_bounding_box(position1) / 2.f;
  const float other_r_squared = dot(other_bonding_box, other_bonding_box);
  const vec2 my_bonding_box = get_bounding_box(position2) / 2.f;
  const float my_r_squared = dot(my_bonding_box, my_bonding_box);
  const float r_squared = max(other_r_squared, my_r_squared);
  if (dist_squared < r_squared)
    return true;
  return false;
}

// Axis-Aligned Bounding Box Collision detection.
bool box_collides(const Position &position1, const Position &position2) {
  vec2 bounding_box1 = get_bounding_box(position1);
  float horizontal_dist_box1 = (bounding_box1.x) / 2.0f;
  float vertical_dist_box1 = (bounding_box1.y) / 2.0f;

  float left_bound_box1 = position1.position.x - horizontal_dist_box1;
  float right_bound_box1 = position1.position.x + horizontal_dist_box1;
  float top_bound_box1 = position1.position.y - vertical_dist_box1;
  float bot_bound_box1 = position1.position.y + vertical_dist_box1;

  vec2 bounding_box2 = get_bounding_box(position2);
  float horizontal_dist_box2 = (bounding_box2.x) / 2.0f;
  float vertical_dist_box2 = (bounding_box2.y) / 2.0f;

  float left_bound_box2 = position2.position.x - horizontal_dist_box2;
  float right_bound_box2 = position2.position.x + horizontal_dist_box2;
  float top_bound_box2 = position2.position.y - vertical_dist_box2;
  float bot_bound_box2 = position2.position.y + vertical_dist_box2;

  bool vertical_overlap = top_bound_box1 < bot_bound_box2;
  bool vertical_overlap2 = top_bound_box2 < bot_bound_box1;
  bool horizontal_overlap = left_bound_box1 < right_bound_box2;
  bool horizontal_overlap2 = left_bound_box2 < right_bound_box1;

  return vertical_overlap && vertical_overlap2 && horizontal_overlap &&
         horizontal_overlap2;
}

void PhysicsSystem::step(float elapsed_ms) {
  auto &motion_registry = registry.motions;
  auto &position_registry = registry.positions;
  for (uint i = 0; i < motion_registry.size(); i++) {
    Entity entity = motion_registry.entities[i];
    Motion &motion = motion_registry.get(entity);
    Position &position = position_registry.get(entity);

    float step_seconds = elapsed_ms / 1000.f;
    vec2 distance = (motion.velocity) * step_seconds;
    position.position += distance;
  }

  collision_detection();
  collision_resolution();
}

// Collision Detection System
void PhysicsSystem::collision_detection() {
  ComponentContainer<Position> &position_container = registry.positions;
  for (uint i = 0; i < position_container.components.size(); i++) {
    Position &position_i = position_container.components[i];
    Entity entity_i = position_container.entities[i];

    for (uint j = i + 1; j < position_container.components.size(); j++) {
      Position &position_j = position_container.components[j];
      Entity entity_j = position_container.entities[j];

      /********** Collisions to Ignore *************/

      // no collision component
      if (!registry.collidables.has(entity_i) ||
          !registry.collidables.has(entity_j)) {
        continue;
      }

      // wall to wall
      if (registry.activeWalls.has(entity_i) &&
          registry.activeWalls.has(entity_j)) {
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

      // None of the above, then properly detect collisions.
      if (box_collides(position_i, position_j)) {
        registry.collisions.emplace_with_duplicates(entity_i, entity_j);
        registry.collisions.emplace_with_duplicates(entity_j, entity_i);
      }
    }
  }
}

void PhysicsSystem::collision_resolution_debug_info(Entity entity,
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

void PhysicsSystem::collision_resolution() {
  auto &collisionsRegistry = registry.collisions;
  for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
    Entity entity = collisionsRegistry.entities[i];
    Entity entity_other = collisionsRegistry.components[i].other;

    // collision_resolution_debug_info(entity, entity_other);

    // Player Collision Handling
    if (registry.players.has(entity)) {
      resolvePlayerCollisions(entity, entity_other);
    }

    // Wall Collision Handling
    if (registry.activeWalls.has(entity)) {
      resolveWallCollisions(entity, entity_other);
    }

    // Player Projectile Collision Handling
    if (registry.playerProjectiles.has(entity)) {
      resolvePlayerProjCollisions(entity, entity_other);
    }

    // Remove all collisions from this simulation step
    registry.collisions.clear();
  }
}

void PhysicsSystem::resolvePlayerCollisions(Entity player, Entity other) {

  // Player - Enemy
  if (registry.deadlys.has(other)) {
    handle_debuffs(player, other);
    modifyOxygen(player, other);

    registry.sounds.insert(player, Sound(death_sound));
  } else if (registry.consumables.has(other)) {
    // Player - Consumable
    handle_consumable_collisions(player, other);
  } else if (registry.interactable.has(other)) {
    // Checking Player - interactable collisions
    handle_interactable_collisions(player, other);
  }
}

void PhysicsSystem::resolveWallCollisions(Entity wall, Entity other) {
  if (!registry.motions.has(other)) {
    return;
  }

  Motion &other_motion = registry.motions.get(other);
  // Wall - Player
  if (registry.players.has(other)) {
    Position &wall_position = registry.positions.get(wall);
    Position &player_position = registry.positions.get(other);

    vec2 bounding_box1 = get_bounding_box(wall_position);
    float horizontal_dist_wall = (bounding_box1.x) / 2.0f;
    float vertical_dist_wall = (bounding_box1.y) / 2.0f;

    float left_bound_wall = wall_position.position.x - horizontal_dist_wall;
    float right_bound_wall = wall_position.position.x + horizontal_dist_wall;
    float top_bound_wall = wall_position.position.y - vertical_dist_wall;
    float bot_bound_wall = wall_position.position.y + vertical_dist_wall;

    vec2 bounding_box2 = get_bounding_box(player_position);
    float horizontal_dist_player = (bounding_box2.x) / 2.0f;
    float vertical_dist_player = (bounding_box2.y) / 2.0f;

    float left_bound_player =
        player_position.position.x - horizontal_dist_player;
    float right_bound_player =
        player_position.position.x + horizontal_dist_player;
    float top_bound_player = player_position.position.y - vertical_dist_player;
    float bot_bound_player = player_position.position.y + vertical_dist_player;

    float players_right_overlaps_left_wall =
        (right_bound_player - left_bound_wall);
    float players_left_overlaps_right_wall =
        (right_bound_wall - left_bound_player);
    float players_bot_overlaps_top_wall = (bot_bound_player - top_bound_wall);
    float players_top_overlaps_bot_wall = (bot_bound_wall - top_bound_player);

    float overlapX =
        min(players_right_overlaps_left_wall, players_left_overlaps_right_wall);
    float overlapY =
        min(players_bot_overlaps_top_wall, players_top_overlaps_bot_wall);
    if (overlapX < overlapY) {
      overlapX = (player_position.position.x < wall_position.position.x)
                     ? -1 * overlapX
                     : overlapX;
      player_position.position.x += overlapX;
    } else {
      overlapY = (player_position.position.y < wall_position.position.y)
                     ? -1 * overlapY
                     : overlapY;
      player_position.position.y += overlapY;
    }
    other_motion.velocity = vec2(0.0f, 0.0f);
  }

  // Wall - Projectile
  if (registry.playerProjectiles.has(other)) {
    other_motion.velocity = vec2(0.0, 0.0);
    registry.playerProjectiles.get(other).is_loaded = true;
  }

  // Wall - Enemy
  if (registry.deadlys.has(other)) {
    other_motion.velocity = other_motion.velocity * -1.0f;
  }
}

void PhysicsSystem::resolvePlayerProjCollisions(Entity player_proj,
                                                Entity other) {
  DamageOnTouch &proj_dmg = registry.damageTouch.get(player_proj);
  Motion &player_proj_motion = registry.motions.get(player_proj);
  PlayerProjectile &player_proj_comp =
      registry.playerProjectiles.get(player_proj);

  // Player Projectile - Enemy
  if (registry.deadlys.has(other)) {
    Oxygen &enemy_oxygen = registry.oxygen.get(other);
    enemy_oxygen.level -= proj_dmg.amount;

    player_proj_motion.velocity = vec2(0.0, 0.0);
    player_proj_comp.is_loaded = true;
  }
}

void updateWepProjPos(vec2 mouse_pos, Entity player, Entity player_weapon,
                      Entity player_projectile) {
  vec2 player_pos = registry.positions.get(player).position;
  vec2 pos_cursor_vec = mouse_pos - player_pos;
  float angle = atan2(pos_cursor_vec.y, pos_cursor_vec.x);
  Position &weapon_pos = registry.positions.get(player_weapon);
  Position &proj_pos = registry.positions.get(player_projectile);
  weapon_pos.angle = angle;
  weapon_pos.position = calculate_pos_vec(GUN_RELATIVE_POS_FROM_PLAYER.x,
                                          player_pos, weapon_pos.angle);
  if (registry.playerProjectiles.get(player_projectile).is_loaded) {
    proj_pos.angle = angle;
    proj_pos.position = calculate_pos_vec(
        HARPOON_RELATIVE_POS_FROM_GUN.x, weapon_pos.position, proj_pos.angle,
        {0.f, HARPOON_RELATIVE_POS_FROM_GUN.y});
  }
}

void setFiredProjVelo(Entity player_projectile) {
  PlayerProjectile &proj = registry.playerProjectiles.get(player_projectile);
  proj.is_loaded = false;
  float angle = registry.positions.get(player_projectile).angle;
  registry.motions.get(player_projectile).velocity = {
      HARPOON_SPEED * cos(angle), HARPOON_SPEED * sin(angle)};
  registry.sounds.insert(player_projectile, Sound(blast_sound));
}

void setPlayerAcceleration(Entity player) {
  Motion &motion = registry.motions.get(player);
  Player &keys = registry.players.get(player);
  motion.acceleration = {0.f, 0.f};

  // If player is dashing, double acceleration
  float accel_inc = registry.players.get(player).dashing ? DASH_ACCELERATION
                                                         : PLAYER_ACCELERATION;

  if (keys.upHeld) {
    motion.acceleration.y -= accel_inc;
  }
  if (keys.downHeld) {
    motion.acceleration.y += accel_inc;
  }
  if (keys.leftHeld) {
    motion.acceleration.x -= accel_inc;
  }
  if (keys.rightHeld) {
    motion.acceleration.x += accel_inc;
  }
}

void calculatePlayerVelocity(Entity player, float lerp) {
  Motion &motion = registry.motions.get(player);

  if (!debuff_entity_can_move(player)) {
    motion.velocity = {0.f, 0.f};
    return;
  }

  motion.velocity += motion.acceleration * lerp;

  if (abs(motion.acceleration.x) == WATER_FRICTION &&
      abs(motion.velocity.x) < abs(motion.acceleration.x * lerp) &&
      motion.velocity.x * motion.acceleration.x > 0) {
    motion.velocity.x = 0;
  }
  if (abs(motion.acceleration.y) == WATER_FRICTION &&
      abs(motion.velocity.y) < abs(motion.acceleration.y * lerp) &&
      motion.velocity.y * motion.acceleration.y > 0) {
    motion.velocity.y = 0;
  }

  // If player is dashing, double max speed
  float max_velocity =
      registry.players.get(player).dashing ? MAX_DASH_SPEED : MAX_PLAYER_SPEED;

  if (motion.velocity.x > max_velocity) {
    motion.velocity.x = max_velocity;
  } else if (motion.velocity.x < -max_velocity) {
    motion.velocity.x = -max_velocity;
  }
  if (motion.velocity.y > max_velocity) {
    motion.velocity.y = max_velocity;
  } else if (motion.velocity.y < -max_velocity) {
    motion.velocity.y = -max_velocity;
  }
}

void applyWaterFriction(Entity entity) {
  Motion &motion = registry.motions.get(entity);

  // Water friction should accelerate in the opposite direction of the player's
  // velocity. If the player isn't moving, friction has no effect
  if (motion.velocity.x) {
    motion.velocity.x > 0 ? motion.acceleration.x -= WATER_FRICTION
                          : motion.acceleration.x += WATER_FRICTION;
  }
  if (motion.velocity.y) {
    motion.velocity.y > 0 ? motion.acceleration.y -= WATER_FRICTION
                          : motion.acceleration.y += WATER_FRICTION;
  }
}
