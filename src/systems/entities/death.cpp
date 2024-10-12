#include "death.hpp"
#include "debuff.hpp"

bool update_death(float elapsed_ms_since_last_update) {
  for (Entity entity : registry.deathTimers.entities) {
    // progress timer
    DeathTimer &counter = registry.deathTimers.get(entity);
    counter.counter_ms -= elapsed_ms_since_last_update;

    // restart the game once the death timer expired
    if (counter.counter_ms < 0) {
      registry.deathTimers.remove(entity);
      if (registry.players.has(entity)) {
        return true;
      }
    }
  }
  return false;
}
