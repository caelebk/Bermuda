
#include "tiny_ecs_registry.hpp"

/**
 * @brief adds stunned to the player if the player isn't already stunned and the
 * enemy is able to stun
 *
 * @param enemy
 * @param player
 * @return true - if the entity wasn't stunning the player (so the player isn't constantly being damaged)
 */
bool handle_stun(Entity enemy, Entity player) {
  if (registry.stuns.has(enemy)) {
    if (!registry.stunned.has(player)) {
      registry.stunned.emplace(player);
    }
    return false;
  }
  return true;
}
