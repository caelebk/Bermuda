#include "oxygen_system.hpp"

#include <iostream>

#include "audio_system.hpp"
#include "enemy_factories.hpp"
#include "player_factories.hpp"
#include "tiny_ecs_registry.hpp"
#include "world_system.hpp"

///////////////////////////////////////////////////////////////////////////////
// OXYGEN SYSTEM
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief reduces entity's oxygen from idle/basic movement, registers death
 * NOTE: Possibly modified for damage over time in the future
 */
void depleteOxygen(Entity& entity) {
  if (!registry.oxygen.has(entity)) {
    return;
  }
  Oxygen& entity_oxygen = registry.oxygen.get(entity);
  entity_oxygen.level += calcDeltaOxygen(entity_oxygen, entity_oxygen.rate);
  updateHealthBarRender(entity, entity_oxygen, entity_oxygen.rate);
  updateOxygenLvlStatus(entity_oxygen);
  updateDeathStatus(entity, entity_oxygen);

  if (registry.players.has(entity) && !registry.deathTimers.has(entity)) {
    registry.sounds.insert(Entity(), Sound(deplete_audio));
    if (registry.lowOxygen.has(entity_oxygen.oxygenBar)) {
      registry.sounds.insert(Entity(), Sound(fast_heart_audio));
    } else {
      registry.sounds.insert(Entity(), Sound(slow_heart_audio));
    }
  }
}

/**
 * @brief modifies oxygen from an oxygenModifier, registers death
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
 * @param oxygenModifier - oxygenModifier's amount is (+) if refilling,
 *                                                    (-) if damaging/costly
 */
void modifyOxygen(Entity& entity, Entity& oxygenModifier) {
  if (registry.deathTimers.has(entity) || !registry.oxygen.has(entity) ||
      !registry.oxygenModifiers.has(oxygenModifier) ||
      isModOnCooldown(oxygenModifier)) {
    return;
  }
  Oxygen& entity_oxygen = registry.oxygen.get(entity);
  float   oxyModAmount  = registry.oxygenModifiers.get(oxygenModifier).amount;
  float   deltaOxygen   = calcDeltaOxygen(entity_oxygen, oxyModAmount);
  entity_oxygen.level += deltaOxygen;
  updateHealthBarRender(entity, entity_oxygen, deltaOxygen);
  updateOxygenLvlStatus(entity_oxygen);
  updateDeathStatus(entity, entity_oxygen);

  // play hurt sound if player is damaged AND not dead
  if (registry.players.has(entity) &&
      !registry.playerWeapons.has(oxygenModifier) &&
      !registry.deathTimers.has(entity) && deltaOxygen <= 0) {
    registry.sounds.insert(Entity(), Sound(hurt_sound));
  }
}

/**
 * @brief update the position values of an enemy's health bar depending on enemy
 * position
 */
void updateEnemyHealthBarPos(Entity& enemy) {
  Position& enemyPos = registry.positions.get(enemy);
  Oxygen&   enemyOxygen =
      registry.oxygen.get(enemy);  // TODO: ensure no unexpected behaviour once
                                   // collision handling is fully implemented

  Position& oxygenBarPos = registry.positions.get(enemyOxygen.oxygenBar);
  Position& backgroundBarPos =
      registry.positions.get(enemyOxygen.backgroundBar);

  oxygenBarPos.position =
      enemyPos.position - vec2(0.f, enemyPos.scale.y / 2 + ENEMY_O2_BAR_GAP);
  backgroundBarPos.position =
      enemyPos.position - vec2(0.f, enemyPos.scale.y / 2 + ENEMY_O2_BAR_GAP);
}

///////////////////////////////////////////////////////////////////////////////
// HELPERS AND WRAPPERS
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief wrapper for oxygen depletion
 */
float oxygen_drain(Entity player, float oxygen_deplete_timer,
                   float elapsed_ms_since_last_update) {
  oxygen_deplete_timer -= elapsed_ms_since_last_update;
  if (oxygen_deplete_timer < 0) {
    depleteOxygen(player);
    return PLAYER_OXYGEN_DEPLETE_TIME_MS;
  }
  return oxygen_deplete_timer;
}

/**
 * @brief updates and/or checks if oxygenModifier is on a cooldown
 *
 * @param oxygenModifier
 *
 * @return true if oxygenModifier is on a cooldown, otherwise false
 */
bool isModOnCooldown(Entity& oxygenModifier) {
  if (registry.modifyOxygenCd.has(oxygenModifier)) {
    auto& modifyOxygenCd = registry.modifyOxygenCd.get(oxygenModifier);
    if (modifyOxygenCd.curr_cd > 0.f) {
      return true;
    }
    modifyOxygenCd.curr_cd = modifyOxygenCd.default_cd;
  }
  return false;
}

/**
 * @brief calculate correct deltaOxygen
 *
 * @param entity_oxygen: entity's oxygen component
 * @param oxygenModifierAmount: refill (+) or damage/cost (-) amount
 */
float calcDeltaOxygen(Oxygen& entity_oxygen, float oxygenModifierAmount) {
  return min(oxygenModifierAmount,
             entity_oxygen.capacity - entity_oxygen.level);
}

/**
 * @brief apply changes to entity's health bar (or oxygen tank) rendering
 *
 * @param entity
 * @param entity_oxygen
 * @param deltaOxygen: refill (+) or damage/cost (-) amount
 */
void updateHealthBarRender(Entity& entity, Oxygen& entity_oxygen,
                           float deltaOxygen) {
  if (!(registry.positions.has(entity_oxygen.oxygenBar) &&
        registry.positions.has(entity_oxygen.backgroundBar))) {
    return;
  }
  Position& barPositionComponent =
      registry.positions.get(entity_oxygen.oxygenBar);
  vec2& barOriginalScale = barPositionComponent.originalScale;
  vec2& barScale         = barPositionComponent.scale;
  vec2& barPosition      = barPositionComponent.position;
  bool  isPlayer         = registry.players.has(entity);
  float barPercentChange = (deltaOxygen / entity_oxygen.capacity);
  float deltaBarScale    = (isPlayer) ? barPercentChange * barOriginalScale.y
                                      : barPercentChange * barOriginalScale.x;

  float& scaleToChange    = (isPlayer) ? barScale.y : barScale.x;
  float& positionToChange = (isPlayer) ? barPosition.y : barPosition.x;
  scaleToChange           = max(0.f, scaleToChange + deltaBarScale);
  positionToChange        = (isPlayer) ? positionToChange - deltaBarScale / 2
                                       : positionToChange + deltaBarScale / 2;
}

/**
 * @brief apply changes to entity's oxygen level status
 *
 * @param entity_oxygen
 */
void updateOxygenLvlStatus(Oxygen& entity_oxygen) {
  if (entity_oxygen.level / entity_oxygen.capacity <= LOW_OXYGEN_THRESHOLD &&
      !registry.lowOxygen.has(entity_oxygen.oxygenBar)) {
    registry.lowOxygen.emplace(entity_oxygen.oxygenBar);
  } else if (entity_oxygen.level / entity_oxygen.capacity >
                 LOW_OXYGEN_THRESHOLD &&
             registry.lowOxygen.has(entity_oxygen.oxygenBar)) {
    registry.lowOxygen.remove(entity_oxygen.oxygenBar);
  }
}

/**
 * @brief apply changes to reflect an entity's death if needed
 *
 * @param entity
 * @param entity_oxygen
 */
void updateDeathStatus(Entity& entity, Oxygen& entity_oxygen) {
  // if entity is not dead, return
  if (entity_oxygen.level > 0) {
    return;
  }
  // if entity is dead, ensure no negative health
  entity_oxygen.level = 0;

  if (!registry.deathTimers.has(entity) && registry.players.has(entity)) {
    registry.deathTimers.insert(entity, {4000.f});
    registry.sounds.insert(Entity(), Sound(death_sound));
    registry.sounds.insert(Entity(), Sound(flat_line_sound));
  } else if (!registry.deathTimers.has(entity) && !registry.players.has(entity)) {
      registry.deathTimers.emplace(entity);
    }
}
