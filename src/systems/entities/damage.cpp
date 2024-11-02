#include "damage.hpp"

#include "debuff.hpp"
#include "tiny_ecs_registry.hpp"

/**
 * @brief Updates all attacked timers
 *
 * @return true if successful
 */
bool update_collision_timers(float elapsed_ms_since_last_update) {
  for (Entity entity : registry.attacked.entities) {
    // progress timer
    Attacked& timer = registry.attacked.get(entity);
    timer.timer -= elapsed_ms_since_last_update;

    // remove if no longer stunned
    if (timer.timer < 0) {
      registry.attacked.remove(entity);
    }
  }
  return true;
}

void addDamageIndicatorTimer(Entity entity) {
  if (!registry.attacked.has(entity)) {
    Attacked& attacked = registry.attacked.emplace(entity);
    attacked.timer     = DEFAULT_COLLISION_INDICATOR_TIMER;
  } else {
    Attacked& attacked = registry.attacked.get(entity);
    attacked.timer     = DEFAULT_COLLISION_INDICATOR_TIMER;
  }
}