#include "physics_system.hpp"

#include <cstdio>
#include <iostream>
#include <world_system.hpp>

#include "audio_system.hpp"
#include "consumable_utils.hpp"
#include "debuff.hpp"
#include "enemy_util.hpp"
#include "map_util.hpp"
#include "oxygen_system.hpp"
#include "physics.hpp"
#include "player_factories.hpp"
#include "tiny_ecs_registry.hpp"

void PhysicsSystem::step(float elapsed_ms) {
  // Calculate 't value': time loop / loop duration
  float lerp = elapsed_ms / LOOP_DURATION;

  // Set player acceleration (If player is alive)
  if (!registry.deathTimers.has(player)) {
    setPlayerAcceleration();
  } else if (registry.motions.has(player)) {
    registry.motions.get(player).acceleration = {0.f, 0.f};
  }

  // Apply Water friction
  applyWaterFriction(player);

  // Apply dash if player is dashing or increment cooldown for dash
  playerDash();

  // Update player velocity with lerp
  if (!registry.players.get(player).dashing) {
    calculatePlayerVelocity(lerp);
  }

  // Update Entity positions with lerp
  for (Entity entity : registry.motions.entities) {
    Motion&   motion   = registry.motions.get(entity);
    Position& position = registry.positions.get(entity);
    position.position += motion.velocity * lerp;

    if (!debuff_entity_can_move(entity)) {
      motion.velocity = vec2(0.0f);
    }

    if (debuff_entity_knockedback(entity)) {
      KnockedBack& knockedback = registry.knockedback.get(entity);
      motion.velocity          = knockedback.knocked_velocity;
    }

    if (registry.oxygen.has(entity) && entity != player) {
      // make sure health bars follow moving enemies
      updateEnemyHealthBarPos(entity);
    }

    if (registry.emoting.has(entity)) {
      updateEmotePos(entity);
    }
  }
}

void updateWepProjPos(vec2 mouse_pos) {
  Position& player_comp    = registry.positions.get(player);
  vec2      player_pos     = player_comp.position;
  vec2      pos_cursor_vec = mouse_pos - player_pos;
  vec2      arm_offset     = (player_comp.scale.x < 0)
                                 ? vec2(-ARM_OFFSET.x, ARM_OFFSET.y)
                                 : ARM_OFFSET;
  pos_cursor_vec -= arm_offset;
  float     angle      = atan2(pos_cursor_vec.y, pos_cursor_vec.x);
  Position& weapon_pos = registry.positions.get(player_weapon);
  Position& proj_pos   = registry.positions.get(player_projectile);
  weapon_pos.angle     = (player_comp.scale.x < 0) ? angle + 3.14159 : angle;

  float flipped = (player_comp.scale.x < 0) ? -1 : 1;
  switch (wep_type) {
    case (PROJECTILES::HARPOON):
      weapon_pos.position =
          calculate_pos_vec(GUN_RELATIVE_POS_FROM_PLAYER.x * flipped,
                            player_pos, weapon_pos.angle, arm_offset);
      break;
    case (PROJECTILES::NET):
      weapon_pos.position =
          calculate_pos_vec(NET_GUN_RELATIVE_POS_FROM_PLAYER.x * flipped,
                            player_pos, weapon_pos.angle, arm_offset);
      break;
    case (PROJECTILES::CONCUSSIVE):
      weapon_pos.position =
          calculate_pos_vec(CONCUSSIVE_GUN_RELATIVE_POS_FROM_PLAYER.x * flipped,
                            player_pos, weapon_pos.angle, arm_offset);
      break;
    case (PROJECTILES::TORPEDO):
      weapon_pos.position =
          calculate_pos_vec(TORPEDO_GUN_RELATIVE_POS_FROM_PLAYER.x * flipped,
                            player_pos, weapon_pos.angle, arm_offset);
      break;
    case (PROJECTILES::SHRIMP):
      weapon_pos.position =
          calculate_pos_vec(SHRIMP_GUN_RELATIVE_POS_FROM_PLAYER.x * flipped,
                            player_pos, weapon_pos.angle, arm_offset);
      break;
  }

  if (registry.playerProjectiles.get(player_projectile).is_loaded) {
    vec2 relative_pos = HARPOON_RELATIVE_POS_FROM_GUN;
    switch (wep_type) {
      case (PROJECTILES::NET):
        relative_pos = NET_RELATIVE_POS_FROM_GUN;
        break;
      case (PROJECTILES::CONCUSSIVE):
        relative_pos   = CONCUSSIVE_RELATIVE_POS_FROM_GUN;
        proj_pos.scale = proj_pos.originalScale;
        if (registry.positions.get(player).scale.x < 0) {
          proj_pos.scale.x = -proj_pos.scale.x;
        }
        break;
      case (PROJECTILES::TORPEDO):
        relative_pos = TORPEDO_RELATIVE_POS_FROM_GUN;
        break;
      case (PROJECTILES::SHRIMP):
        relative_pos = SHRIMP_RELATIVE_POS_FROM_GUN;
        break;
      case (PROJECTILES::PROJ_COUNT):
        break;
    }
    if (player_comp.scale.x < 0) {
      relative_pos.x *= -1;
    }
    proj_pos.angle = weapon_pos.angle;
    proj_pos.position =
        calculate_pos_vec(relative_pos.x, weapon_pos.position, proj_pos.angle,
                          {0.f, relative_pos.y});
  }
}

void updatePlayerDirection(vec2 mouse_pos) {
  Position& player_pos = registry.positions.get(player);
  bool      mouse_right_face_left =
      player_pos.position.x < mouse_pos.x && player_pos.scale.x < 0;
  bool mouse_left_face_right =
      player_pos.position.x > mouse_pos.x && player_pos.scale.x > 0;
  if (mouse_right_face_left || mouse_left_face_right) {
    player_pos.scale.x *= -1;

    Position& weapon = registry.positions.get(player_weapon);
    weapon.scale.x *= -1;
    weapon.position.x *= -1;

    Position& projectile = registry.positions.get(player_projectile);
    projectile.scale.x *= -1;
    projectile.position.x *= -1;
  }
}

void setFiredProjVelo() {
  PlayerProjectile& proj = registry.playerProjectiles.get(player_projectile);
  proj.is_loaded         = false;
  float   angle          = registry.positions.get(player_projectile).angle;
  Motion& proj_motion    = registry.motions.get(player_projectile);
  vec2&   proj_scale     = registry.positions.get(player_projectile).scale;
  vec2&   proj_original_scale =
      registry.positions.get(player_projectile).originalScale;
  float direction = registry.positions.get(player).scale.x /
                    abs(registry.positions.get(player).scale.x);
  switch (proj.type) {
    case PROJECTILES::SHRIMP:
      proj_motion.velocity = {SHRIMP_SPEED * cos(angle) * direction,
                              SHRIMP_SPEED * sin(angle) * direction};
      break;
    case PROJECTILES::CONCUSSIVE:
      proj_scale = vec2(5.f) * proj_original_scale;
      if (registry.positions.get(player).scale.x < 0) {
        proj_scale.x = -proj_scale.x;
      }
    default:
      proj_motion.velocity = {HARPOON_SPEED * cos(angle) * direction,
                              HARPOON_SPEED * sin(angle) * direction};
      break;
  }
  registry.sounds.insert(player_projectile, Sound(blast_sound));
}

void setPlayerAcceleration() {
  Motion& motion      = registry.motions.get(player);
  Player& keys        = registry.players.get(player);
  motion.acceleration = {0.f, 0.f};

  // If player is dashing, double acceleration
  float accel_inc = registry.players.get(player).gliding ? GLIDE_ACCELERATION
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

void calculatePlayerVelocity(float lerp) {
  Motion& motion = registry.motions.get(player);

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

  // If player is gliding, double max speed
  float max_velocity =
      registry.players.get(player).gliding ? MAX_GLIDE_SPEED : MAX_PLAYER_SPEED;

  Player& player_comp = registry.players.get(player);
  // player either activated dash or is in the middle of dash
  if (player_comp.dashing || player_comp.dashTimer > 0) {
    playerDash();
  } else {
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
}

void playerDash() {
  Motion& motion = registry.motions.get(player);
  Player& keys   = registry.players.get(player);

  if (keys.dashTimer > 0) {
    keys.dashTimer -= 1;
  }

  if (keys.dashCooldownTimer > 0) {
    keys.dashCooldownTimer -= 1;
    return;
  }

  bool activated_and_can_dash = keys.dashing && keys.dashCooldownTimer <= 0;
  if (!activated_and_can_dash) {
    return;
  }

  registry.sounds.insert(Entity(), Sound(blast_sound));
  keys.dashCooldownTimer = DASH_COOLDOWN_DURATION;
  keys.dashTimer         = DASH_DURATION;

  if (keys.upHeld) {
    motion.velocity.y = -1 * DASH_SPEED;
  }
  if (keys.downHeld) {
    motion.velocity.y = DASH_SPEED;
  }
  if (keys.leftHeld) {
    motion.velocity.x = -1 * DASH_SPEED;
  }
  if (keys.rightHeld) {
    motion.velocity.x = DASH_SPEED;
  }

  keys.dashing = false;
}

void applyWaterFriction(Entity entity) {
  Motion& motion = registry.motions.get(entity);

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
