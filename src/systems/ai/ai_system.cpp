// internal
#include "ai_system.hpp"

#include <cstdio>

#include "ai.hpp"
#include "collision_system.hpp"
#include "enemy_factories.hpp"
#include "enemy_util.hpp"
#include "physics.hpp"
#include "random.hpp"
#include "tiny_ecs.hpp"
#include <debuff.hpp>


#include "tiny_ecs_registry.hpp"

static inline bool is_tracking(Entity e) {
  if (registry.trackPlayer.has(e)) {
    return registry.trackPlayer.get(e).active_track;
  }
  return false;
}

static inline bool is_proj(Entity e) {
  return registry.actsAsProjectile.has(e);
}



static void removeFromAI(Entity &e) {
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
}


void AISystem::do_boss_ai(float elapsed_ms) {
  for (Entity &b: registry.bosses.entities) {
    Boss &boss = registry.bosses.get(b);
    boss.curr_cd -= elapsed_ms;

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
 * @brief Does a ray cast to see if the vector from the position to the player
 * intersects with any walls
 *
 * @param pos
 * @return
 */
bool AISystem::can_see_player(Position& pos, Position& player_pos) {
  vec2        direction   = player_pos.position - pos.position;
  const float player_dist = dot(direction, direction);
  direction               = normalize(direction);

  // iterate through all walls since its equally has hard to determine if the
  // wall is between or not
  for (Entity w : registry.activeWalls.entities) {
    if (!registry.positions.has(w)) {
      continue;
    }

    const Position& wall_pos = registry.positions.get(w);

    const vec2  wall_dir_ent     = wall_pos.position - pos.position;
    const float wall_dist_ent    = dot(wall_dir_ent, wall_dir_ent);
    const vec2  wall_dir_player  = wall_pos.position - player_pos.position;
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
    float t_min_x = (left - player_pos.position.x) / direction.x;
    float t_max_x = (right - player_pos.position.x) / direction.x;
    float t_min_y = (bottom - player_pos.position.y) / direction.y;
    float t_max_y = (top - player_pos.position.y) / direction.y;

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
    // w, {TEXTURE_ASSET_ID::PLAYER, EFFECT_ASSET_ID::TEXTURED,
    // GEOMETRY_BUFFER_ID::SPRITE});
    return false;
  }

  return true;
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
        !can_see_player(entity_pos, player_pos)) {
      if (tracker.active_track) {
        printf("%d stopped tracking the player!\n", (unsigned int)e);
        createEmote(this->renderer, e, EMOTE::QUESTION);
      }
      tracker.active_track = false;
      continue;
    }
    printf("%d is tracking the player!\n", (unsigned int)e);
    if (tracker.active_track) {
      createEmote(this->renderer, e, EMOTE::NONE);
    } else {
      createEmote(this->renderer, e, EMOTE::EXCLAMATION);
    }
    tracker.active_track = true;

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
        !can_see_player(entity_pos, player_pos)) {
      if (tracker.active_track) {
        printf("%d stopped tracking the player!\n", (unsigned int)e);
        createEmote(this->renderer, e, EMOTE::QUESTION);
      }
      tracker.active_track = false;
      continue;
    }
    printf("%d is tracking the player!\n", (unsigned int)e);
    if (tracker.active_track) {
      createEmote(this->renderer, e, EMOTE::NONE);
    } else {
      createEmote(this->renderer, e, EMOTE::EXCLAMATION);
    }
    tracker.active_track = true;

    // set the entity velocity
    Motion& motion     = registry.motions.get(e);
    float   velocity   = sqrt(dot(motion.velocity, motion.velocity));
    vec2    player_dir_o = player_pos.position - entity_pos.position;
    vec2    player_dir = normalize(player_dir_o);

    motion.velocity     = player_dir * velocity;
    motion.acceleration = player_dir * tracker.acceleration;

    // if the player is too close try to kite, move away from player
    float player_dist = sqrt(dot(player_dir_o, player_dir_o));
    if (player_dist <= tracker.min_distance)  {
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

    // TODO: make this generic for different projectiles, for now just create a fish

    // create a fish overlapping them
    Entity fish = createFishPos(this->renderer, entity_pos.position);
    // make them pretend that they're a projectile
    registry.actsAsProjectile.emplace(fish);

    // make them go towards the player's current direction
    Motion &fish_motion = registry.motions.get(fish);
    float fish_velocity = sqrt(dot(fish_motion.velocity, fish_motion.velocity)) * 2;
    float fish_accel = sqrt(dot(fish_motion.acceleration, fish_motion.acceleration)) * 2;
    fish_motion.velocity = fish_velocity * player_dir;
    fish_motion.acceleration = fish_accel * player_dir;
  }
}


void AISystem::step(float elapsed_ms) {
  // minibosses
  if (registry.bosses.entities.size() > 0) {
    do_boss_ai(elapsed_ms);
  }

  do_wander_ai(elapsed_ms);
  do_wander_ai_line(elapsed_ms);
  do_wander_ai_square(elapsed_ms);
  do_track_player(elapsed_ms);
  do_track_player_ranged(elapsed_ms);
}

void AISystem::init(RenderSystem* renderer_arg) {
  this->renderer = renderer_arg;
}
