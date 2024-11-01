
#include "debuff.hpp"

#include <iostream>

#include "abilities.hpp"
#include "tiny_ecs_registry.hpp"

/**
 * @brief adds stunned to the player if the player isn't already stunned and the
 * enemy is able to stun
 *
 * @param stun_entity
 * @param stunned_entity
 * @return true if success
 */
bool handle_stun(Entity stun_entity, Entity stunned_entity) {
  if (registry.stuns.has(stun_entity)) {
    if (!registry.stunned.has(stunned_entity)) {
      std::cout << "Stunned!" << std::endl;
      Stun&    stun    = registry.stuns.get(stun_entity);
      Stunned& stunned = registry.stunned.emplace(stunned_entity);
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
bool debuff_entity_can_move(Entity& entity) {
  if (registry.stunned.has(entity)) {
    Stunned& stunned = registry.stunned.get(entity);
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
    Stunned& stunned = registry.stunned.get(entity);
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
 * @param debuffed_entity
 * @param debuff_entity
 * @return
 */
bool handle_debuffs(Entity debuffed_entity, Entity debuff_entity) {
  bool success = true;

  success &= handle_stun(debuff_entity, debuffed_entity);

  return success;
}
