
#include "debuff.hpp"

#include <iostream>

#include "abilities.hpp"
#include "tiny_ecs_registry.hpp"
#include "player_hud.hpp"
#include <player_controls.hpp>

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
      if (registry.deadlys.has(stunned_entity)) {
        stunned.original_velocity = registry.motions.has(stunned_entity) ? registry.motions.get(stunned_entity).velocity : vec2(0.f);
      }
    }
    return false;
  }
  return true;
}

bool handle_knockback(Entity knockback_entity, Entity knockedback_entity) {
  if (registry.knockbacks.has(knockback_entity)) {
    if (!registry.knockedback.has(knockedback_entity)) {
      std::cout << "KnockedBack!" << std::endl;
      KnockBack&    knockback    = registry.knockbacks.get(knockback_entity);
      KnockedBack& knockedBack = registry.knockedback.emplace(knockedback_entity);
      knockedBack.duration = knockback.duration;
      knockedBack.knockback_proj = knockback_entity;

      bool knockedback_has_motion = registry.motions.has(knockedback_entity);
      bool knockback_has_motion = registry.motions.has(knockback_entity);

      knockedBack.knocked_velocity = knockback_has_motion ? 
                                      vec2(KNOCKBACK_MULTIPLIER) * registry.motions.get(knockback_entity).velocity : 
                                      vec2(0.0f);
      knockedBack.original_velocity = knockedback_has_motion ? 
                                      registry.motions.get(knockedback_entity).velocity : 
                                      vec2(0.0f);
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
 * @brief returns true if enemy is being knocked back
 *
 * note: I basically made this for consistency purposes.
 * @param entity
 * @return true if the entity is being knocked back, false otherwise
 */
bool debuff_entity_knockedback(Entity& entity) {
  return registry.knockedback.has(entity);
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
      if (stunned.original_velocity != vec2(0.f)) {
        if (registry.motions.has(entity) && registry.deadlys.has(entity)) {
          registry.motions.get(entity).velocity = stunned.original_velocity;
        }
      }
      registry.stunned.remove(entity);
    }
  }
  ///////////////////////
  // Knockback
  ///////////////////////
  for (Entity entity : registry.knockedback.entities) {
    // progress timer
    KnockedBack& knockedback = registry.knockedback.get(entity);
    knockedback.duration -= elapsed_ms_since_last_update;

    // remove knocked back debuff
    if (knockedback.duration <= 0) {
      if (registry.motions.has(entity)) {
        registry.motions.get(entity).velocity = knockedback.original_velocity;
      }
      registry.knockedback.remove(entity);

      Inventory& inventory = registry.inventory.get(player);
      PlayerProjectile& playerproj_component = registry.playerProjectiles.get(knockedback.knockback_proj);
      bool check_wep_swap = player_projectile != knockedback.knockback_proj;
      playerproj_component.is_loaded = true;

      if (check_wep_swap) {
        destroyGunOrProjectile(knockedback.knockback_proj);
      }
      if (inventory.concussors <= 0) {
        doWeaponSwap(harpoon, harpoon_gun, PROJECTILES::HARPOON);
        changeSelectedCounterColour(INVENTORY::HARPOON);
      }
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
  success &= handle_knockback(debuff_entity, debuffed_entity);

  return success;
}
