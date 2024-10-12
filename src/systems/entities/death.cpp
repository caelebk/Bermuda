#include "death.hpp"

bool update_death(float elapsed_ms_since_last_update, ScreenState &screen) {
  float min_counter_ms = MIN_COUNTER_MS;
  for (Entity entity : registry.deathTimers.entities) {
    // progress timer
    DeathTimer &counter = registry.deathTimers.get(entity);
    counter.counter_ms -= elapsed_ms_since_last_update;
    if (counter.counter_ms < min_counter_ms) {
      min_counter_ms = counter.counter_ms;
    }

    // restart the game once the death timer expired
    if (counter.counter_ms < 0) {
      registry.deathTimers.remove(entity);
      if (registry.players.has(entity)) {
        screen.darken_screen_factor = 0;
        // reduce window brightness if the player is dying
        // screen.darken_screen_factor = 1 - min_counter_ms / 3000;
        return true;
      }
    }
  }
  return false;
}
