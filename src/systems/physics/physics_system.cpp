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

  // Update player velocity with lerp
  calculatePlayerVelocity(lerp);

  // Update Entity positions with lerp
  for (Entity entity : registry.motions.entities) {
    if (!debuff_entity_can_move(entity)) {
      continue;
    }
    Motion&   motion   = registry.motions.get(entity);
    Position& position = registry.positions.get(entity);
    position.position += motion.velocity * lerp;
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
  vec2      player_pos     = registry.positions.get(player).position;
  vec2      pos_cursor_vec = mouse_pos - player_pos;
  float     angle          = atan2(pos_cursor_vec.y, pos_cursor_vec.x);
  Position& weapon_pos     = registry.positions.get(player_weapon);
  Position& proj_pos       = registry.positions.get(player_projectile);
  weapon_pos.angle         = angle;
  weapon_pos.position      = calculate_pos_vec(GUN_RELATIVE_POS_FROM_PLAYER.x,
                                               player_pos, weapon_pos.angle);
  if (registry.playerProjectiles.get(player_projectile).is_loaded) {
    vec2 relative_pos = HARPOON_RELATIVE_POS_FROM_GUN;
    switch (wep_type) {
      case ((int)PROJECTILES::NET):
        relative_pos = NET_RELATIVE_POS_FROM_GUN;
    }

    proj_pos.angle = angle;
    proj_pos.position =
        calculate_pos_vec(relative_pos.x, weapon_pos.position, proj_pos.angle,
                          {0.f, relative_pos.y});
  }
}

void setFiredProjVelo() {
  PlayerProjectile& proj = registry.playerProjectiles.get(player_projectile);
  proj.is_loaded         = false;
  float angle            = registry.positions.get(player_projectile).angle;
  registry.motions.get(player_projectile).velocity = {
      HARPOON_SPEED * cos(angle), HARPOON_SPEED * sin(angle)};
  registry.sounds.insert(player_projectile, Sound(blast_sound));
}

void setPlayerAcceleration() {
  Motion& motion      = registry.motions.get(player);
  Player& keys        = registry.players.get(player);
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

void calculatePlayerVelocity(float lerp) {
  Motion& motion = registry.motions.get(player);

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
