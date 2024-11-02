#include "tiny_ecs_registry.hpp"

#define DEFAULT_COLLISION_INDICATOR_TIMER 300.f

/**
 * @brief Updates all attacked timers
 *
 * @return true if successful
 */
bool update_collision_timers(float elapsed_ms_since_last_update);

// adds entity to timer if it isn't already in it
void addDamageIndicatorTimer(Entity entity);