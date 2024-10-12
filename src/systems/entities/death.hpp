#include "common.hpp"
#include "common.hpp"
#include "components.hpp"
#include "render_system.hpp"
#include "tiny_ecs_registry.hpp"

#define MIN_COUNTER_MS 3000.0

/**
 * @brief Updates all death timers
 *
 * @param elapsed_ms_since_last_update 
 * @return true if the player is dead, false otherwise
 */
bool update_death(float elapsed_ms_since_last_update, ScreenState& screen);
