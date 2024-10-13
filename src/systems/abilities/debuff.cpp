
#include "debuff.hpp"
#include "abilities.hpp"
#include "tiny_ecs_registry.hpp"
#include <iostream>

/**
 * @brief adds stunned to the player if the player isn't already stunned and the
 * enemy is able to stun
 *
 * @param enemy
 * @param player
 * @return true if success
 */
bool handle_stun(Entity enemy, Entity player) {
  if (registry.stuns.has(enemy)) {
    if (!registry.stunned.has(player)) {
      std::cout << "Stunned!" << std::endl;
      Stun &stun = registry.stuns.get(enemy);
      Stunned &stunned = registry.stunned.emplace(player);
      stunned.duration = stun.duration;
    }
    return false;
  }
  return true;
}

/**
 * @brief returns true if there are no debuffs preventing the entity from moving
 *
 * @param entity
 * @return true if the entity can move, false otherwise
 */
bool debuff_entity_can_move(Entity &entity) {
  if (registry.stunned.has(entity)) {
    Stunned &stunned = registry.stunned.get(entity);
    if (stunned.duration >= STUN_MOVEMENT_THRESHOLD_MS) {
      return false;
    }
  }

  return true;
}

/**
 * @brief Updates all debuff timers
 *
 * @return true if successful
 */
bool update_debuffs(float elapsed_ms_since_last_update) {
  ///////////////////////
  // Stun
  ///////////////////////
  for (Entity entity : registry.stunned.entities) {
    // progress timer
    Stunned &stunned = registry.stunned.get(entity);
    stunned.duration -= elapsed_ms_since_last_update;

    // remove if no longer stunned
    if (stunned.duration < 0) {
      registry.stunned.remove(entity);
    }
  }
  return true;
}


/**
 * @brief does all debuff checks that the player would need
 *
 * @param enemy 
 * @param player 
 * @return 
 */
bool handle_debuffs(Entity player, Entity enemy) {
  bool success = true;

  success &= handle_stun(enemy, player);
  
  return success;
}
