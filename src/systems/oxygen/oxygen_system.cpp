#include "oxygen_system.hpp"
#include "enemy_factories.hpp"
#include "player_factories.hpp"
#include "tiny_ecs_registry.hpp"
#include "world_system.hpp"

#include <iostream>

/**
 * @brief reduces player's oxygen from idle/basic movement, registers death
 * NOTE: Possibly modified for damage over time in the future
 */
void depleteOxygen(Entity &entity) {
  if (registry.oxygen.has(entity)) {
    auto &oxygen = registry.oxygen.get(entity);
    oxygen.level -= oxygen.rate;

    if (oxygen.level <= 0) {
      if (!registry.deathTimers.has(entity)) {
        registry.deathTimers.emplace(entity);
        oxygen.level = 0;
      }
    }

    checkAndRenderOxygen(entity, oxygen, oxygen.rate);
  }
}

/**
 * @brief modifies oxygen from an affector, registers death
 * @details tentative plan:
 *  - dash costs 30 per 1s
 *  - firing harpoon costs 25
 *  - various standard weapons cost 30-50
 *  - pistol shrimp costs 100
 *  - non-boss damage can cost anywhere between 20-100
 *  - boss damage can cost 100-500
 *  - geysers fill 100 per 1s
 *  - each oxygen tank fills 50 per 1s for up to 10s (500 max)
 *
 * @param entity
 * @param affector - affector's amount is positive if damaging, negative if
 * refilling
 */
void modifyOxygen(Entity &entity, Entity &affector) {
  if (!registry.oxygen.has(entity) || !registry.damageTouch.has(affector)) {
    return;
  }

  if (registry.attackCD.has(affector)) {
    auto &attackCD = registry.attackCD.get(affector);
    if (attackCD.attack_cd > 0.f) {
      return;
    }
    attackCD.attack_cd = attackCD.attack_spd;
  }

  auto &oxygen = registry.oxygen.get(entity);
  auto &oxygenChange = registry.damageTouch.get(affector);

  if (isDeadAfterChange(oxygen, oxygenChange.amount)) {
    if (!registry.deathTimers.has(entity)) {
      registry.deathTimers.emplace(entity);
    }
    // return if enemy; we don't render an enemy with 0 health
    if (registry.deadlys.has(entity))
      return;
  }

  checkAndRenderOxygen(entity, oxygen, oxygenChange.amount);
}

// helper for entity taking damage from affector
// NOTE: amount is POSITIVE if damaging, NEGATIVE if refilling
// return true if entity dies, false otherwise
bool isDeadAfterChange(Oxygen &oxygen, float amount) {
  oxygen.level = min(oxygen.level - amount, oxygen.capacity);

  if (oxygen.level <= 0) {
    oxygen.level = 0;
    return true;
  }
  return false;
}

// helper for checking and rendering oxygen bar
void checkAndRenderOxygen(Entity &entity, Oxygen &oxygen, float amount) {
  checkOxygenLevel(entity); // TODO: consider adding at end of function

  // no change needed if we are already at capacity
  if (oxygen.level == oxygen.capacity) {
    return;
  }

  // modify oxygen bar
  if (registry.positions.has(oxygen.oxygenBar) &&
      registry.positions.has(oxygen.backgroundBar)) {
    auto &oxygenBarPos = registry.positions.get(oxygen.oxygenBar);
    bool isPlayer = registry.players.has(entity);
    float scaledAmount =
        (isPlayer) ? (amount / oxygen.capacity) * oxygenBarPos.originalScale.y
                   : (amount / oxygen.capacity) * oxygenBarPos.originalScale.x;
    float &scale = (isPlayer) ? oxygenBarPos.scale.y : oxygenBarPos.scale.x;
    float &position =
        (isPlayer) ? oxygenBarPos.position.y : oxygenBarPos.position.x;

    if (scale - scaledAmount < 0) { // prevent negative scaling
      scaledAmount = scale;
    }
    scale -= scaledAmount;
    position =
        (isPlayer) ? position + scaledAmount / 2 : position - scaledAmount / 2;
  }
}

/**
 * @brief checks if player oxygen is low and adds/removes lowOxygen component
 * accordingly
 */
void checkOxygenLevel(Entity &entity) {
  if (registry.oxygen.has(entity)) { // TODO: REMOVE second cond if we want
    auto &oxygen =
        registry.oxygen.get(entity); // enemy health to change colours

    if (oxygen.level / oxygen.capacity <= LOW_OXYGEN_THRESHOLD) {
      if (!registry.lowOxygen.has(oxygen.oxygenBar)) {
        registry.lowOxygen.emplace(oxygen.oxygenBar);
      }
    } else {
      if (registry.lowOxygen.has(oxygen.oxygenBar)) {
        registry.lowOxygen.remove(oxygen.oxygenBar);
      }
    }
  }
}

/**
 * @brief update the position values of an enemy's health bar depending on enemy
 * position
 */
void updateHealthBarAndEnemyPos(Entity &enemy) {
  Position &enemyPos = registry.positions.get(enemy);
  Oxygen &enemyOxygen =
      registry.oxygen.get(enemy); // TODO: ensure no unexpected behaviour once
                                  // collision handling is fully implemented

  Position &oxygenBarPos = registry.positions.get(enemyOxygen.oxygenBar);
  Position &backgroundBarPos =
      registry.positions.get(enemyOxygen.backgroundBar);

  oxygenBarPos.position =
      enemyPos.position - vec2(0.f, enemyPos.scale.y / 2 + ENEMY_O2_BAR_GAP);
  backgroundBarPos.position =
      enemyPos.position - vec2(0.f, enemyPos.scale.y / 2 + ENEMY_O2_BAR_GAP);
}

float oxygen_drain(Entity player, float oxygen_deplete_timer,
                   float elapsed_ms_since_last_update) {
  oxygen_deplete_timer -= elapsed_ms_since_last_update;
  if (oxygen_deplete_timer < 0) {
    depleteOxygen(player);
    return PLAYER_OXYGEN_DEPLETE_TIME_MS;
  }
  return oxygen_deplete_timer;
}
