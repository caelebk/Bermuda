#include "boids.hpp"

#include <cstdio>
#include <glm/geometric.hpp>
#include <unordered_set>

#include "ai.hpp"
#include "ai_system.hpp"
#include "collision_system.hpp"
#include "debuff.hpp"
#include "physics.hpp"
#include "physics_system.hpp"
#include "random.hpp"
#include "tiny_ecs.hpp"
#include "tiny_ecs_registry.hpp"
#include "world_system.hpp"

static vec2 get_center_of_mass(Group& g) {
  vec2 result = {0.f, 0.f};

  for (Entity e : g.members) {
    if (!registry.positions.has(e)) {
      continue;
    }

    Position& pos = registry.positions.get(e);
    result += pos.position;
  }
  result.x = result.x / (float)g.members.size();
  result.y = result.y / (float)g.members.size();

  return result;
}

static vec2 get_avg_dir(Group& g) {
  vec2 result = {0.f, 0.f};

  for (Entity e : g.members) {
    if (!registry.motions.has(e)) {
      continue;
    }

    Motion& motion = registry.motions.get(e);
    result += motion.velocity;
  }
  result.x = result.x / (float)g.members.size();
  result.y = result.y / (float)g.members.size();

  return normalize(result);
}

static inline float get_speed(Entity e) {
  if (registry.motions.has(e)) {
    Motion& motion = registry.motions.get(e);
    return sqrt(dot(motion.velocity, motion.velocity));
  }
  return 0.f;
}

static void do_boid_separation(Group& g, Entity e) {
  if (!registry.positions.has(e) || !registry.motions.has(e)) {
    return;
  }
  vec2      dir_vec  = {0.f, 0.f};
  Position& position = registry.positions.get(e);
  Motion&   motion   = registry.motions.get(e);

  // get average direction of all group members within range
  for (Entity other : g.members) {
    if (e == other || !registry.positions.has(other)) {
      continue;
    }
    Position& pos_other = registry.positions.get(other);

    vec2  local_dir = position.position - pos_other.position;
    float dist      = sqrt(dot(local_dir, local_dir));

    if (dist <= MIN_DIST) {
      dir_vec += local_dir;
    }
  }
  motion.velocity += dir_vec * SEPERATION_WEIGHT;
}

static void do_boid_avoid_obstacles(Entity e) {
  // get average direction of all group members within range
  if (registry.positions.has(e)) {
    return;
  }
  vec2      dir_vec  = {0.f, 0.f};
  Position& position = registry.positions.get(e);
  Motion&   motion   = registry.motions.get(e);
  for (Entity wall : registry.activeWalls.entities) {
    if (!registry.positions.has(wall)) {
      continue;
    }
    Position& pos_other = registry.positions.get(wall);
    vec2      point     = find_closest_point(position, pos_other);

    vec2  local_dir = position.position - point;
    float dist      = sqrt(dot(local_dir, local_dir));

    if (dist <= MIN_DIST) {
      dir_vec += local_dir;
    }
  }
  motion.velocity += dir_vec * SEPERATION_WEIGHT;
}

static void do_boid_alignment(Group& g, Entity e) {
  if (is_tracking(e)) {
    // do not disturb entities that are chasing the player
    return;
  }

  if (!registry.motions.has(e)) {
    return;
  }
  vec2 avg_dir = get_avg_dir(g);
  avg_dir /= ALIGNMENT_WEIGHT;

  Motion& motion = registry.motions.get(e);
  motion.velocity += avg_dir;
}

static void do_boid_cohesion(Group& g, Entity e) {
  if (is_tracking(e)) {
    // do not disturb entities that are chasing the player
    return;
  }

  if (!registry.positions.has(e) || !registry.motions.has(e)) {
    return;
  }
  vec2 center_of_mass = get_center_of_mass(g);

  Position& position = registry.positions.get(e);
  Motion&   motion   = registry.motions.get(e);

  vec2 dir = (center_of_mass - position.position) * COHESION_WEIGHT;

  motion.velocity += dir;
}

static void do_tracking_surround(Group& g) {
  const int  left   = 1;
  const int  right  = 2;
  const mat2 rot45  = mat2(0.7071, -0.7071, 0.7071, 0.7071);
  const mat2 rotN45 = mat2(0.7071, 0.7071, -0.7071, 0.7071);

  int behaviour_counter = 0;
  // int behaviour_count   = 3;

  const Position& player_pos = registry.positions.get(player);
  for (Entity e : g.members) {
    if (!is_tracking(e) || !registry.positions.has(e) ||
        !registry.motions.has(e)) {
      continue;
    }
    // reset cooldown so it looks less jank
    EntityGroup& eg  = registry.entityGroups.get(e);
    eg.active_dir_cd = eg.change_dir_cd;

    Position& enemy_pos    = registry.positions.get(e);
    Motion&   enemy_motion = registry.motions.get(e);

    //
    // first shark moves directly at the player
    // 2nd and 3rd always move 45 and -45 degrees towards the player respectively
    // rest of the sharks fan out between 60 and -60 degrees
    vec2 player_dir = player_pos.position - enemy_pos.position;
    if (behaviour_counter > right) {
      float theta = randomFloat(-1.05, 1.05);
      float c = cos(theta);
      float s = sin(theta);
      const mat2 rot = mat2(c, -s, s, c);
      player_dir = rot * player_dir;
    } else if (behaviour_counter == left) {
      player_dir = rot45 * player_dir;
    } else if (behaviour_counter == right) {
      player_dir = rotN45 * player_dir;
    }

    float speed = sqrt(dot(enemy_motion.velocity, enemy_motion.velocity));
    enemy_motion.velocity = player_dir;
    do_boid_avoid_obstacles(e);
    enemy_motion.velocity = normalize(enemy_motion.velocity) * speed;

    enemy_pos.scale.x = abs(enemy_pos.scale.x);
    if (enemy_motion.velocity.x > 0) {
      enemy_pos.scale.x *= -1.0f;
    }

    behaviour_counter++;
  }
}

static void do_normalize_speed(Entity e, float speed) {
  if (speed == 0.f) {
    return;
  }

  if (registry.motions.has(e) && registry.positions.has(e)) {
    Position& position = registry.positions.get(e);
    Motion&   motion   = registry.motions.get(e);
    motion.velocity    = normalize(motion.velocity) * speed;

    position.scale.x = abs(position.scale.x);
    if (motion.velocity.x > 0) {
      position.scale.x *= -1.0f;
    }
  }
}

void do_boids(float elapsed_ms) {
  for (Group& g : registry.groups.components) {
    // update individual entities within the boid
    for (Entity e : g.members) {
      if (!registry.entityGroups.has(e)) {
        continue;
      }

      EntityGroup& eg = registry.entityGroups.get(e);
      eg.active_dir_cd -= elapsed_ms;
      if (eg.active_dir_cd <= 0.f) {
        float speed = get_speed(e);
        do_boid_avoid_obstacles(e);
        do_boid_separation(g, e);
        do_boid_alignment(g, e);
        do_boid_cohesion(g, e);
        do_normalize_speed(e, speed);
        eg.active_dir_cd = eg.change_dir_cd;
      }
    }

    // collaborative behaviour
    g.active_dir_cd -= elapsed_ms;
    if (g.active_dir_cd <= 0.f) {
      do_tracking_surround(g);
      g.active_dir_cd = 500.f;
    }
  }
}
