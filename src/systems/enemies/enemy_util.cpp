#include "enemy_util.hpp"

#include <glm/common.hpp>

#include "enemy.hpp"
#include "oxygen.hpp"
#include "tiny_ecs_registry.hpp"

/**
 * @brief updates all enemies
 *
 * @return
 */
bool update_attack(float elapsed_time_ms) {
  for (Entity& e : registry.modifyOxygenCd.entities) {
    ModifyOxygenCD& attackCd = registry.modifyOxygenCd.get(e);
    attackCd.curr_cd = max(attackCd.curr_cd - elapsed_time_ms, 0.f);
  }

  return true;
}
