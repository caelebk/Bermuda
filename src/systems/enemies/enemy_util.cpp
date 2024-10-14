#include "enemy_util.hpp"

#include <glm/common.hpp>

#include "enemy.hpp"
#include "tiny_ecs_registry.hpp"

/**
 * @brief updates all enemies
 *
 * @return
 */
bool update_attack(float elapsed_time_ms) {
  for (Entity& e : registry.attackCD.entities) {
    AttackCD& attackCd = registry.attackCD.get(e);
    attackCd.attack_cd = max(attackCd.attack_cd - elapsed_time_ms, 0.f);
  }

  return true;
}
