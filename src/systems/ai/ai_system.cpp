// internal
#include "ai_system.hpp"

#include <cstdio>
#include <debuff.hpp>
#include <vector>

#include "ai.hpp"
#include "boids.hpp"
#include "boss_factories.hpp"
#include "collision_system.hpp"
#include "collision_util.hpp"
#include "enemy_factories.hpp"
#include "enemy_util.hpp"
#include "entity_type.hpp"
#include "physics.hpp"
#include "random.hpp"
#include "tiny_ecs.hpp"
#include "tiny_ecs_registry.hpp"

bool is_tracking(Entity e) {
  if (registry.trackPlayer.has(e)) {
    return registry.trackPlayer.get(e).active_track;
  }

  if (registry.trackPlayerRanged.has(e)) {
    return registry.trackPlayerRanged.get(e).active_track;
  }
  return false;
}

bool any_tracking(std::vector<Entity> entities) {
  if (entities.size() < 1) {
    return false;
  }

  if (!registry.trackPlayer.has(entities[0]) &&
      !registry.trackPlayerRanged.has(entities[0])) {
    return false;
  }

  for (Entity e : entities) {
    if (is_tracking(e)) {
      return true;
    }
  }

  return false;
}

static inline bool is_proj(Entity e) {
  return registry.actsAsProjectile.has(e);
}

void removeFromAI(Entity& e) {
  if (registry.wanders.has(e)) {
    registry.wanders.remove(e);
  }

  if (registry.wanderLines.has(e)) {
    registry.wanderLines.remove(e);
  }

  if (registry.wanderSquares.has(e)) {
    registry.wanderSquares.remove(e);
  }

  if (registry.trackPlayer.has(e)) {
    registry.trackPlayer.remove(e);
  }

  if (registry.trackPlayerRanged.has(e)) {
    registry.trackPlayerRanged.remove(e);
  }

  if (registry.shooters.has(e)) {
    registry.shooters.remove(e);
  }
}

/**
 * @brief Does a ray cast to see if the vector from the position to the player
 * intersects with any walls
 *
 * @param pos
 * @return
 */
bool can_see_entity(Position& pos, Position& entity_pos) {
  vec2        direction   = entity_pos.position - pos.position;
  const float player_dist = dot(direction, direction);
  direction               = normalize(direction);

  // iterate through all walls since its equally as hard to determine if the
  // wall is between or not
  for (Entity w : registry.activeWalls.entities) {
    if (!registry.positions.has(w)) {
      continue;
    }

    const Position& wall_pos = registry.positions.get(w);

    const vec2  wall_dir_ent     = wall_pos.position - pos.position;
    const float wall_dist_ent    = dot(wall_dir_ent, wall_dir_ent);
    const vec2  wall_dir_player  = wall_pos.position - entity_pos.position;
    const float wall_dist_player = dot(wall_dir_player, wall_dir_player);

    // wall is further than player, ignore
    if (wall_dist_ent > player_dist || wall_dist_player > player_dist) {
      continue;
    }

    vec4 wall_bounds = get_bounds(wall_pos);

    float left   = wall_bounds[0];
    float right  = wall_bounds[1];
    float top    = wall_bounds[2];
    float bottom = wall_bounds[3];

    // Calculate t for intersections on each axis
    float t_min_x = (left - entity_pos.position.x) / direction.x;
    float t_max_x = (right - entity_pos.position.x) / direction.x;
    float t_min_y = (bottom - entity_pos.position.y) / direction.y;
    float t_max_y = (top - entity_pos.position.y) / direction.y;

    // Sort t values on each axis
    float t1_x = min(t_min_x, t_max_x);
    float t2_x = max(t_min_x, t_max_x);
    float t1_y = min(t_min_y, t_max_y);
    float t2_y = max(t_min_y, t_max_y);

    // Find the largest t_min and smallest t_max
    float t_min = max(t1_x, t1_y);
    float t_max = min(t2_x, t2_y);

    // not intersecting
    // not intersecting
    if (t_min > t_max) {
      continue;
    }

    // debug, turns walls in between to the player texture
    // registry.renderRequests.remove(w);
    // registry.renderRequests.insert(
    // w, {TEXTURE_ASSET_ID::JELLY, EFFECT_ASSET_ID::TEXTURED,
    // GEOMETRY_BUFFER_ID::SPRITE});
    return false;
  }

  return true;
}

void choose_new_direction(Entity enemy, Entity other) {
  if (!registry.positions.has(enemy) || !registry.positions.has(other)) {
    return;
  }

  vec2 direction = registry.positions.get(player).position -
                   registry.positions.get(enemy).position;
  float distance = sqrt(dot(direction, direction));
  float speed    = sqrt(dot(registry.motions.get(enemy).velocity,
                            registry.motions.get(enemy).velocity));
  Boss& boss     = registry.bosses.get(enemy);
  // if enemy sees player during target mode, aim at player
  if (registry.trackPlayer.has(enemy) &&
      can_see_entity(registry.positions.get(enemy),
                     registry.positions.get(player)) &&
      distance <= registry.trackPlayer.get(enemy).spot_radius) {
    // turn red
    boss.is_angry                                = true;
    registry.trackPlayer.get(enemy).active_track = true;
  } else {
    if (registry.trackPlayer.has(enemy)) {
      registry.trackPlayer.get(enemy).active_track = false;
    }
    // revert to normal color
    boss.is_angry      = false;
    vec2 closest_point = find_closest_point(registry.positions.get(enemy),
                                            registry.positions.get(other));

    direction = registry.positions.get(enemy).position - closest_point;
    if (direction.x == 0) {
      direction.x = randomFloat(-speed, speed);
    } else {
      direction.y = randomFloat(-speed, speed);
    }
  }

  if (direction.x < 0) {
    registry.positions.get(enemy).scale.x =
        abs(registry.positions.get(enemy).scale.x) * -1;
  } else {
    registry.positions.get(enemy).scale.x =
        abs(registry.positions.get(enemy).scale.x);
  }

  direction                            = normalize(direction);
  registry.motions.get(enemy).velocity = direction * speed;
}

void handleUrchinFiring(RenderSystem* renderer, Position& pos) {
  launchUrchinNeedle(renderer, pos.position + vec2(abs(pos.scale.x), 0), 0);
  launchUrchinNeedle(renderer, pos.position + vec2(0, abs(pos.scale.y)),
                     M_PI / 2);
  launchUrchinNeedle(renderer, pos.position + vec2(-abs(pos.scale.x), 0), M_PI);
  launchUrchinNeedle(renderer, pos.position + vec2(0, -abs(pos.scale.y)),
                     1.5f * M_PI);
}

void handleCthulhuRageProjs(RenderSystem* renderer, Position& pos,
                            float targetAngle) {
  // shoot one proj at player, 8 in selectively random directions
  shootRageProjectile(renderer, pos.position, targetAngle);

  shootRageProjectile(renderer, pos.position, randomFloat(0, M_PI / 4));
  shootRageProjectile(renderer, pos.position, randomFloat(M_PI / 4, M_PI / 2));
  shootRageProjectile(renderer, pos.position,
                      randomFloat(M_PI / 2, 3 * M_PI / 4));
  shootRageProjectile(renderer, pos.position, randomFloat(3 * M_PI / 4, M_PI));
  shootRageProjectile(renderer, pos.position, randomFloat(M_PI, 5 * M_PI / 4));
  shootRageProjectile(renderer, pos.position,
                      randomFloat(5 * M_PI / 4, 3 * M_PI / 2));
  shootRageProjectile(renderer, pos.position,
                      randomFloat(3 * M_PI / 2, 7 * M_PI / 4));
  shootRageProjectile(renderer, pos.position,
                      randomFloat(7 * M_PI / 4, 2 * M_PI));
}

void AISystem::do_boss_ai(float elapsed_ms) {
  for (Entity& b : registry.bosses.entities) {
    Boss& boss = registry.bosses.get(b);
    boss.curr_cd -= elapsed_ms;

    if (boss.type == ENTITY_TYPE::SHARKMAN) {
      sharkman_texture_num += 0.075f;
      if (sharkman_texture_num >= 8.f) {
        sharkman_texture_num = 0.f;
      }
      auto& renderReq = registry.renderRequests.get(b);
      // switch? nah
      if (sharkman_texture_num < 1) {
        renderReq.used_texture = TEXTURE_ASSET_ID::SHARKMAN0;
      } else if (sharkman_texture_num < 2) {
        renderReq.used_texture = TEXTURE_ASSET_ID::SHARKMAN1;
      } else if (sharkman_texture_num < 3) {
        renderReq.used_texture = TEXTURE_ASSET_ID::SHARKMAN2;
      } else if (sharkman_texture_num < 4) {
        renderReq.used_texture = TEXTURE_ASSET_ID::SHARKMAN3;
      } else if (sharkman_texture_num < 5) {
        renderReq.used_texture = TEXTURE_ASSET_ID::SHARKMAN4;
      } else if (sharkman_texture_num < 6) {
        renderReq.used_texture = TEXTURE_ASSET_ID::SHARKMAN5;
      } else if (sharkman_texture_num < 7) {
        renderReq.used_texture = TEXTURE_ASSET_ID::SHARKMAN6;
      } else {
        renderReq.used_texture = TEXTURE_ASSET_ID::SHARKMAN7;
      }
    }

    if (boss.curr_cd > 0) {
      continue;
    }
    boss.curr_cd = boss.ai_cd;

    // removes their current ai
    removeFromAI(b);

    // add random ai
    auto ai_func = boss.ai[getRandInt(0, boss.ai.size() - 1)];
    ai_func();
  }
}

/**
 * @brief updates all entities that are wandering. this will randomly change
 * their direction
 *
 * @param elapsed_ms
 */
void AISystem::do_wander_ai(float elapsed_ms) {
  for (Entity& e : registry.wanders.entities) {
    if (is_tracking(e) || is_proj(e)) {
      continue;
    }

    Wander& wander = registry.wanders.get(e);
    wander.active_dir_cd -= elapsed_ms;

    if (wander.active_dir_cd > 0) {
      continue;
    }

    if (!debuff_entity_can_move(e)) {
      continue;
    }

    wander.active_dir_cd = wander.change_dir_cd;

    if (!registry.motions.has(e)) {
      continue;
    }

    createEmote(this->renderer, e, EMOTE::NONE);
    Motion& motion       = registry.motions.get(e);
    float   speed        = sqrt(dot(motion.velocity, motion.velocity));
    float   acceleration = sqrt(dot(motion.acceleration, motion.acceleration));
    float   newAngle     = randomFloat(0.f, 2 * 3.14);
    motion.velocity      = {cos(newAngle), sin(newAngle)};
    motion.acceleration  = {cos(newAngle), sin(newAngle)};
    motion.velocity *= speed;
    motion.acceleration *= acceleration;

    if (registry.positions.has(e)) {
      Position& p = registry.positions.get(e);

      p.scale.x = abs(p.scale.x);
      if (motion.velocity.x > 0) {
        // scale should be opposite of velocity
        p.scale.x *= -1;
      }
    }
  }
}
/**
 * @brief updates all entities that are wandering. this will randomly change
 * their direction to the other line direction
 *
 * @param elapsed_ms
 */
void AISystem::do_wander_ai_line(float elapsed_ms) {
  for (Entity& e : registry.wanderLines.entities) {
    if (is_tracking(e) || is_proj(e)) {
      continue;
    }
    WanderLine& wander = registry.wanderLines.get(e);
    wander.active_dir_cd -= elapsed_ms;

    if (wander.active_dir_cd > 0) {
      continue;
    }
    wander.active_dir_cd = wander.change_dir_cd;

    if (!registry.motions.has(e)) {
      continue;
    }

    createEmote(this->renderer, e, EMOTE::NONE);
    Motion& motion = registry.motions.get(e);
    motion.velocity *= -1;
    motion.acceleration *= -1;

    if (registry.positions.has(e)) {
      Position& p = registry.positions.get(e);

      p.scale.x = abs(p.scale.x);
      if (motion.velocity.x > 0) {
        // scale should be opposite of velocity
        p.scale.x *= -1;
      }
    }
  }
}

/**
 * @brief updates all entities that are wandering. this will randomly change
 * their direction to the other line direction
 *
 * @param elapsed_ms
 */
void AISystem::do_wander_ai_square(float elapsed_ms) {
  // 90 degree rotations
  static const mat2 rotateClockwise        = mat2(0.0, 1.0, -1.0, 0.0);
  static const mat2 rotateCounterClockwise = mat2(0.0, -1.0, 1.0, 0.0);

  for (Entity& e : registry.wanderSquares.entities) {
    if (is_tracking(e) || is_proj(e)) {
      continue;
    }
    WanderSquare& wander = registry.wanderSquares.get(e);
    wander.active_dir_cd -= elapsed_ms;

    if (wander.active_dir_cd > 0) {
      continue;
    }
    wander.active_dir_cd = wander.change_dir_cd;

    if (!registry.motions.has(e)) {
      continue;
    }

    createEmote(this->renderer, e, EMOTE::NONE);
    Motion& motion = registry.motions.get(e);
    if (wander.clockwise) {
      motion.velocity     = rotateClockwise * motion.velocity;
      motion.acceleration = rotateClockwise * motion.acceleration;
    } else {
      motion.velocity     = rotateCounterClockwise * motion.velocity;
      motion.acceleration = rotateCounterClockwise * motion.acceleration;
    }

    if (registry.positions.has(e)) {
      Position& p = registry.positions.get(e);

      p.scale.x = abs(p.scale.x);
      if (motion.velocity.x > 0) {
        // scale should be opposite of velocity
        p.scale.x *= -1;
      }
    }
  }
}

/**
 * @brief returns true if position is in range of the player
 *
 * @param pos
 * @return
 */
bool AISystem::in_range_of_player(Position& pos, Position& player_pos,
                                  float range) {
  vec2 distance = player_pos.position - pos.position;
  // printf("%f, %f\n", sqrt(dot(distance, distance)), range);
  return sqrt(dot(distance, distance)) <= range;
}

/**
 * @brief updates all entities that are wandering. this will randomly change
 * their direction to the other line direction
 *
 * @param elapsed_ms
 */
void AISystem::do_track_player(float elapsed_ms) {
  if (registry.players.entities.size() != 1) {
    return;
  }
  Entity player = registry.players.entities[0];

  if (!registry.positions.has(player)) {
    return;
  }

  Position& player_pos = registry.positions.get(player);

  for (Entity& e : registry.trackPlayer.entities) {
    TracksPlayer& tracker = registry.trackPlayer.get(e);
    tracker.curr_cd -= elapsed_ms;

    if (registry.lobsters.has(e)) {
      update_lobster(elapsed_ms, e);
    }

    if (tracker.curr_cd > 0 || is_proj(e)) {
      continue;
    }
    tracker.curr_cd = tracker.tracking_cd;

    if (!registry.positions.has(e)) {
      continue;
    }
    Position& entity_pos = registry.positions.get(e);
    float     range =
        tracker.active_track ? tracker.leash_radius : tracker.spot_radius;

    if (!in_range_of_player(entity_pos, player_pos, range) ||
        !can_see_entity(entity_pos, player_pos)) {
      if (tracker.active_track) {
        // printf("%d stopped tracking the player!\n", (unsigned int)e);
        createEmote(this->renderer, e, EMOTE::QUESTION);
      }
      tracker.active_track = false;
      if (registry.lobsters.has(e)) {
        registry.motions.get(e).velocity =
            vec2(registry.lobsters.get(e).original_speed, 0);
      }
      continue;
    }
    // printf("%d is tracking the player!\n", (unsigned int)e);
    if (tracker.active_track) {
      createEmote(this->renderer, e, EMOTE::NONE);
    } else {
      createEmote(this->renderer, e, EMOTE::EXCLAMATION);
    }
    tracker.active_track = true;

    if (registry.lobsters.has(e)) {
      do_lobster(elapsed_ms, e, player);
      continue;
    }

    Motion& motion     = registry.motions.get(e);
    float   velocity   = sqrt(dot(motion.velocity, motion.velocity));
    vec2    player_dir = normalize(player_pos.position - entity_pos.position);

    motion.velocity     = player_dir * velocity;
    motion.acceleration = player_dir * tracker.acceleration;

    if (registry.positions.has(e)) {
      Position& p = registry.positions.get(e);

      p.scale.x = abs(p.scale.x);
      if (motion.velocity.x > 0) {
        // scale should be opposite of velocity
        p.scale.x *= -1;
      }
    }
  }
}

/**
 * @brief updates all entities that are wandering. this will randomly change
 * their direction to the other line direction
 *
 * @param elapsed_ms
 */
void AISystem::do_track_player_ranged(float elapsed_ms) {
  if (registry.players.entities.size() != 1) {
    return;
  }
  Entity player = registry.players.entities[0];

  if (!registry.positions.has(player)) {
    return;
  }

  Position& player_pos = registry.positions.get(player);

  for (Entity& e : registry.trackPlayerRanged.entities) {
    TracksPlayerRanged& tracker = registry.trackPlayerRanged.get(e);
    tracker.curr_cd -= elapsed_ms;

    if (tracker.curr_cd > 0) {
      continue;
    }
    tracker.curr_cd = tracker.tracking_cd;

    if (!registry.positions.has(e)) {
      continue;
    }
    Position& entity_pos = registry.positions.get(e);
    float     range =
        tracker.active_track ? tracker.leash_radius : tracker.spot_radius;

    if (!in_range_of_player(entity_pos, player_pos, range) ||
        !can_see_entity(entity_pos, player_pos)) {
      if (tracker.active_track) {
        // printf("%d stopped tracking the player!\n", (unsigned int)e);
        createEmote(this->renderer, e, EMOTE::QUESTION);
      }
      tracker.active_track = false;
      continue;
    }
    // printf("%d is tracking the player!\n", (unsigned int)e);
    if (tracker.active_track) {
      createEmote(this->renderer, e, EMOTE::NONE);
    } else {
      createEmote(this->renderer, e, EMOTE::EXCLAMATION);
    }
    tracker.active_track = true;

    // set the entity velocity
    Motion& motion       = registry.motions.get(e);
    float   velocity     = sqrt(dot(motion.velocity, motion.velocity));
    vec2    player_dir_o = player_pos.position - entity_pos.position;
    vec2    player_dir   = normalize(player_dir_o);

    motion.velocity     = player_dir * velocity;
    motion.acceleration = player_dir * tracker.acceleration;

    // if the player is too close try to kite, move away from player
    float player_dist = sqrt(dot(player_dir_o, player_dir_o));
    if (player_dist <= tracker.min_distance) {
      motion.acceleration *= -1.f;
      motion.velocity *= -1.f;
    }

    if (registry.positions.has(e)) {
      Position& p = registry.positions.get(e);

      p.scale.x = abs(p.scale.x);
      if (motion.velocity.x > 0) {
        // scale should be opposite of velocity
        p.scale.x *= -1;
      }
    }

    // create a fish overlapping them
    Entity fish = createFishPos(this->renderer, entity_pos.position, false);

    if ((unsigned int)fish == 0) {
      continue;
    }

    // make them pretend that they're a projectile
    registry.actsAsProjectile.emplace(fish);

    // make them go towards the player's current direction
    Motion& fish_motion = registry.motions.get(fish);
    float   fish_velocity =
        sqrt(dot(fish_motion.velocity, fish_motion.velocity)) * 2;
    float fish_accel =
        sqrt(dot(fish_motion.acceleration, fish_motion.acceleration)) * 2;
    fish_motion.velocity     = fish_velocity * player_dir;
    fish_motion.acceleration = fish_accel * player_dir;
  }
}

void AISystem::do_projectile_firing(float elapsed_ms) {
  for (Entity enemy : registry.shooters.entities) {
    Shooter& attrs = registry.shooters.get(enemy);
    attrs.cooldown -= elapsed_ms;

    if (attrs.type == RangedEnemies::URCHIN) {
      if (attrs.cooldown < 0.f) {
        attrs.cooldown = attrs.default_cd;
        registry.sounds.insert(enemy, Sound(SOUND_ASSET_ID::URCHIN));
        handleUrchinFiring(renderer, registry.positions.get(enemy));
      }
    } else if (attrs.type == RangedEnemies::SEAHORSE) {
      Position& enemy_pos  = registry.positions.get(enemy);
      Position& player_pos = registry.positions.get(player);
      if (can_see_entity(enemy_pos, player_pos)) {
        vec2 direction = player_pos.position - enemy_pos.position;
        if (direction.x > 0) {
          enemy_pos.scale.x = abs(enemy_pos.scale.x) * -1;
        } else {
          enemy_pos.scale.x = abs(enemy_pos.scale.x);
        }
        if (attrs.cooldown < 0.f) {
          attrs.cooldown = attrs.default_cd;
          registry.sounds.insert(enemy, Sound(SOUND_ASSET_ID::SEAHORSE));
          fireSeahorseBullet(renderer, enemy_pos.position, direction);
        }
      } else if (attrs.cooldown < 0.f) {
        attrs.cooldown = attrs.default_cd;
      }
    } else if (attrs.type == RangedEnemies::SIREN && attrs.cooldown < 0.f) {
      Position& enemy_pos = registry.positions.get(enemy);
      for (Entity enemy_ally : registry.deadlys.entities) {
        if (enemy_ally == enemy) continue;
        if (!registry.oxygen.has(enemy_ally)) continue;
        Oxygen& enemy_ally_oxygen = registry.oxygen.get(enemy_ally);
        if (enemy_ally_oxygen.level >= enemy_ally_oxygen.capacity) continue;

        Position& enemy_ally_pos = registry.positions.get(enemy_ally);
        if (can_see_entity(enemy_pos, enemy_ally_pos)) {
          vec2 direction = enemy_ally_pos.position - enemy_pos.position;
          fireSirenHeal(renderer, enemy, enemy_pos.position, direction);
        }
      }
      attrs.cooldown = attrs.default_cd;
    } else if (attrs.type == RangedEnemies::CTHULHU_TENTACLE) {
      if (attrs.cooldown < 0.f) {
        attrs.cooldown = attrs.default_cd;

        if (registry.deadlys.entities.size() < CTHULHU_ENEMY_LIMIT) {
          // create tentacle in 1 of 8 locations around cthulhu
          Position&         enemy_pos     = registry.positions.get(enemy);
          float             gap           = 30;
          float             cthulhu_w_gap = abs(enemy_pos.scale.x) / 2 + gap;
          float             cthulhu_h_gap = abs(enemy_pos.scale.y) / 2 + gap;
          float             width         = 500;
          float             height        = 180;
          std::vector<vec2> spawn_locs    = {
              enemy_pos.position + vec2(-width, -height),
              enemy_pos.position + vec2(width, -height),
              enemy_pos.position + vec2(-width, height),
              enemy_pos.position + vec2(width, height),
              enemy_pos.position + vec2(-cthulhu_w_gap, 0),
              enemy_pos.position + vec2(cthulhu_w_gap, 0),

          };
          createTentaclePos(
              renderer, spawn_locs[getRandInt(0, spawn_locs.size() - 1)], true);
        } else {
          // limit reached, change behaviour
          if (registry.bosses.has(enemy)) {
            Boss& boss   = registry.bosses.get(enemy);
            boss.curr_cd = 0;
            printf("enough tenties\n");
          }
        }
      }
    } else if (attrs.type == RangedEnemies::CTHULHU_FIREBALL) {
      if (attrs.cooldown < 0.f) {
        attrs.cooldown = attrs.default_cd;

        Position& enemy_pos  = registry.positions.get(enemy);
        Position& player_pos = registry.positions.get(player);
        vec2      direction  = player_pos.position - enemy_pos.position;
        shootFireball(renderer, enemy_pos.position, direction);
      }
    } else if (attrs.type == RangedEnemies::CTHULHU_CANISTER) {
      if (attrs.cooldown < 0.f) {
        attrs.cooldown = attrs.default_cd;

        Position& enemy_pos  = registry.positions.get(enemy);
        Position& player_pos = registry.positions.get(player);
        vec2      direction  = player_pos.position - enemy_pos.position;
        bool      is_rage    = registry.bosses.get(enemy).is_angry;
        shootCanister(renderer, enemy_pos.position, direction, is_rage);
      }
    } else if (attrs.type == RangedEnemies::CTHULHU_SHOCKWAVE) {
      if (attrs.cooldown < 0.f) {
        attrs.cooldown      = attrs.default_cd;
        Position& enemy_pos = registry.positions.get(enemy);
        shootShockwave(renderer, enemy_pos.position);
      }
    } else if (attrs.type == RangedEnemies::CTHULHU_RAGE_PROJ) {
      if (attrs.cooldown < 0.f) {
        attrs.cooldown = attrs.default_cd;

        Position& enemy_pos  = registry.positions.get(enemy);
        Position& player_pos = registry.positions.get(player);
        vec2      direction  = player_pos.position - enemy_pos.position;
        handleCthulhuRageProjs(renderer, enemy_pos,
                               atan2(direction.y, direction.x));
      }
    }
  }
}

/**
 * @brief updates all entities that are lobsters. this will randomly change
 * their direction to the other line direction
 *
 * @param elapsed_ms
 */
void AISystem::do_lobster(float elapsed_ms, Entity lobster, Entity player) {
  // printf("DO LOBSTER\n");
  Motion&   lob_motion = registry.motions.get(lobster);
  Position& lob_pos    = registry.positions.get(lobster);
  Lobster&  lob_comp   = registry.lobsters.get(lobster);

  if (lob_comp.ram_timer <= 0 && lob_comp.block_timer <= 0) {
    // printf("LOBSTER START BLOCKING\n");
    if (registry.stunned.has(lobster)) {
      return;
    }
    if (!registry.sounds.has(lobster)) {
      registry.sounds.insert(lobster, Sound(SOUND_ASSET_ID::LOBSTER_SHIELD));
    }
    lob_comp.block_timer    = lob_comp.block_duration;
    lob_motion.velocity     = vec2(0.f);
    lob_motion.acceleration = vec2(0.f);
    if (registry.renderRequests.has(lobster)) {
      RenderRequest& lobster_render = registry.renderRequests.get(lobster);
      lobster_render.used_texture   = TEXTURE_ASSET_ID::LOBSTER_BLOCK;
    }
    return;
  }

  // Position& player_pos   = registry.positions.get(player);
  // float     lob_velocity = sqrt(dot(lob_motion.velocity, lob_motion.velocity));
  // vec2      player_dir   = normalize(player_pos.position - lob_pos.position);

  // lob_motion.velocity     = player_dir * lob_comp.ram_speed;
  // lob_motion.acceleration = player_dir * lob_motion.acceleration;
}

void AISystem::update_lobster(float elapsed_ms, Entity lob) {
  Lobster& lobster = registry.lobsters.get(lob);
  if (lobster.block_timer > 0) {
    // printf("LOBSTER BLOCKING, time: %f\n", lobster.block_timer);
    lobster.block_timer -= elapsed_ms;
    if (lobster.block_timer <= 0) {
      // printf("LOBSTER START RAMMING");
      lobster.ram_timer = lobster.ram_duration;
      if (registry.renderRequests.has(lob)) {
        RenderRequest& lobster_render = registry.renderRequests.get(lob);
        lobster_render.used_texture   = TEXTURE_ASSET_ID::LOBSTER_RAM;
      }
    }
  }
  if (lobster.ram_timer > 0) {
    lobster.ram_timer -= elapsed_ms;
    if (lobster.ram_timer <= 0) {
      // printf("LOBSTER END RAMMING\n");
      if (registry.renderRequests.has(lob)) {
        RenderRequest& lobster_render = registry.renderRequests.get(lob);
        lobster_render.used_texture   = TEXTURE_ASSET_ID::LOBSTER;
      }
    }
  }
  if (registry.positions.has(lob) && registry.motions.has(lob)) {
    Position& p          = registry.positions.get(lob);
    Motion&   lob_motion = registry.motions.get(lob);

    p.scale.x = abs(p.scale.x);
    if (lob_motion.velocity.x > 0) {
      // scale should be opposite of velocity
      p.scale.x *= -1;
    }
  }
}

void AISystem::step(float elapsed_ms) {
  // bosses
  if (registry.bosses.entities.size() > 0) {
    do_boss_ai(elapsed_ms);
  }

  do_wander_ai(elapsed_ms);
  do_wander_ai_line(elapsed_ms);
  do_wander_ai_square(elapsed_ms);
  do_track_player(elapsed_ms);
  do_track_player_ranged(elapsed_ms);
  do_boids(elapsed_ms);
  do_projectile_firing(elapsed_ms);
}

void AISystem::init(RenderSystem* renderer_arg) {
  this->renderer = renderer_arg;
}
