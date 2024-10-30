// internal
#include "ai_system.hpp"
#include <cstdio>
#include "random.hpp"

#include "tiny_ecs.hpp"



/**
 * @brief updates all entities that are wandering. this will randomly change
 * their direction
 *
 * @param elapsed_ms
 */
void AISystem::do_wander_ai(float elapsed_ms) {
  for (Entity& e : registry.wanders.entities) {
    Wander& wander = registry.wanders.get(e);
    wander.active_dir_cd -= elapsed_ms;

    if (wander.active_dir_cd > 0) {
      continue;
    }
    wander.active_dir_cd = wander.change_dir_cd;

    if (!registry.motions.has(e)) {
      continue;
    }

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

void AISystem::step(float elapsed_ms) {
  do_wander_ai(elapsed_ms);
}
