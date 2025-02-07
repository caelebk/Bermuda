#include "oxygen_system.hpp"

#include <cstdio>
#include <iostream>

#include "boss_factories.hpp"
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
    if (registry.lowOxygen.has(entity_oxygen.oxygenBar)) {
      registry.sounds.insert(Entity(),
                             Sound(SOUND_ASSET_ID::PLAYER_FAST_HEART));
    } else {
      registry.sounds.insert(Entity(),
                             Sound(SOUND_ASSET_ID::PLAYER_SLOW_HEART));
    }
  }
}

/**
 * @brief modifies oxygen from an oxygenModifier, registers death
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

  if (!entity_oxygen.isRendered) {
    renderHealthBar(entity_oxygen);
    entity_oxygen.isRendered = true;
  }

  if (registry.lobsters.has(entity)) {
    Lobster& lobster = registry.lobsters.get(entity);
    if (lobster.block_timer > 0) {
      if (!registry.sounds.has(entity)) {
        registry.sounds.insert(entity, Sound(SOUND_ASSET_ID::METAL_CRATE_HIT));
      }
      float block_mitigation = (1.0f - lobster.block_mitigation);
      deltaOxygen = lobster.block_timer > 0 ? block_mitigation * deltaOxygen
                                            : deltaOxygen;
    }
  }

  entity_oxygen.level += deltaOxygen;

  updateHealthBarRender(entity, entity_oxygen, deltaOxygen);
  updateOxygenLvlStatus(entity_oxygen);
  updateDeathStatus(entity, entity_oxygen);

  // play hurt sound if player is damaged, not dashing, AND not dead
  if (registry.players.has(entity) &&
      !registry.playerWeapons.has(oxygenModifier) &&
      !registry.deathTimers.has(entity) &&
      !registry.players.get(entity).dashing && !registry.sounds.has(entity)) {
    if (oxyModAmount <= 0) {
      registry.sounds.insert(entity, Sound(SOUND_ASSET_ID::PLAYER_HURT));
    } else {
      registry.sounds.insert(entity, Sound(SOUND_ASSET_ID::PLAYER_DEPLETE));
    }
  }

  if (registry.breakables.has(entity) && !registry.sounds.has(entity)) {
    bool metal = false;
    if (registry.renderRequests.has(entity)) {
      RenderRequest& request = registry.renderRequests.get(entity);
      metal = request.used_texture == TEXTURE_ASSET_ID::METAL_CRATE;
    }
    if (entity_oxygen.level <= 0) {
      if (metal) {
        registry.sounds.insert(entity,
                               Sound(SOUND_ASSET_ID::METAL_CRATE_DEATH));
      } else {
        registry.sounds.insert(entity, Sound(SOUND_ASSET_ID::CRATE_DEATH));
      }
    } else {
      if (metal) {
        registry.sounds.insert(entity, Sound(SOUND_ASSET_ID::METAL_CRATE_HIT));
      } else {
        registry.sounds.insert(entity, Sound(SOUND_ASSET_ID::CRATE_HIT));
      }
    }
  }
}

void modifyOxygenAmount(Entity& entity, float amount) {
  Oxygen& entity_oxygen = registry.oxygen.get(entity);
  float   deltaOxygen   = calcDeltaOxygen(entity_oxygen, amount);

  if (!entity_oxygen.isRendered) {
    renderHealthBar(entity_oxygen);
    entity_oxygen.isRendered = true;
  }

  entity_oxygen.level += deltaOxygen;
  updateHealthBarRender(entity, entity_oxygen, deltaOxygen);
  updateOxygenLvlStatus(entity_oxygen);
  updateDeathStatus(entity, entity_oxygen);
}

/**
 * @brief update the position values of an entity's health bar depending on
 * entity position
 */
void updateEnemyHealthBarPos(Entity& enemy) {
  Position& enemyPos    = registry.positions.get(enemy);
  Oxygen&   enemyOxygen = registry.oxygen.get(enemy);

  Position& oxygenBarPos = registry.positions.get(enemyOxygen.oxygenBar);
  Position& backgroundBarPos =
      registry.positions.get(enemyOxygen.backgroundBar);

  oxygenBarPos.position =
      enemyPos.position -
      vec2((oxygenBarPos.originalScale.x - oxygenBarPos.scale.x) / 2,
           enemyPos.scale.y / 2 + ENEMY_O2_BAR_GAP);
  backgroundBarPos.position =
      enemyPos.position - vec2(0.f, enemyPos.scale.y / 2 + ENEMY_O2_BAR_GAP);
}

///////////////////////////////////////////////////////////////////////////////
// HELPERS AND WRAPPERS
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief wrapper for oxygen depletion
 */
float oxygen_drain(float oxygen_deplete_timer,
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
    // player death
    registry.deathTimers.insert(entity, {4000.f});
    registry.sounds.insert(Entity(), Sound(SOUND_ASSET_ID::PLAYER_DEATH));
    registry.sounds.insert(Entity(), Sound(SOUND_ASSET_ID::PLAYER_FLATLINE));
    //cheat code for pausing music
    registry.musics.insert(Entity(), MUSIC_ASSET_ID::MUSIC_COUNT);
  } else if (!registry.deathTimers.has(entity) &&
             !registry.players.has(entity)) {
    // cthulu death
    if (registry.bosses.has(entity) &&
        ((registry.bosses.get(entity).type == ENTITY_TYPE::CTHULHU_PHASE1) ||
         (registry.bosses.get(entity).type == ENTITY_TYPE::CTHULHU_PHASE2) ||
         (registry.bosses.get(entity).type == ENTITY_TYPE::CTHULHU_TRANS))) {
      // kill all tentacles
      for (Entity tentacle : registry.deadlys.entities) {
        if (!registry.bosses.has(tentacle)) {
          registry.remove_all_components_of(tentacle);
        }
      }
      // remove all projectiles
      for (Entity projectile : registry.enemyProjectiles.entities) {
        registry.remove_all_components_of(projectile);
      }
      // if cthulhu is angry, he dies, otherwise make him angry
      Boss& cthulhu = registry.bosses.get(entity);
      if (cthulhu.is_angry) {
        // remove render and stop attacks to show its dead
        registry.deathTimers.insert(entity, {3000.f});
        registry.renderRequests.remove(entity);
        registry.shooters.get(entity).cooldown = 10000;
        if (!registry.sounds.has(entity)) {
          registry.sounds.insert(entity,
                                 Sound(SOUND_ASSET_ID::CTHULHU_DEATH, 5000));
        }
      } else {
        // hack, bros ai is healing
        if (registry.deadlys.has(entity)) {
          Deadly& d = registry.deadlys.get(entity);
          d.type    = ENTITY_TYPE::CTHULHU_TRANS;
        }
        cthulhu.type          = ENTITY_TYPE::CTHULHU_TRANS;
        cthulhu.curr_cd       = 0;
        cthulhu.ai_cd         = CTHULHU_REGEN_RATE;
        cthulhu.ai = std::vector<std::function<void()>>({addCthulhuRageAI});
        //pause music during his transition to phase 2.
        registry.musics.insert(Entity(), MUSIC_ASSET_ID::MUSIC_COUNT);
      }
    } else {
      registry.deathTimers.emplace(entity);
      if (!registry.sounds.has(entity) && registry.deadlys.has(entity)) {
        registry.sounds.insert(Entity(), Sound(SOUND_ASSET_ID::ENEMY_DEATH));
      }
      if (registry.bosses.has(entity)) {
        Boss& boss = registry.bosses.get(entity);
        if (boss.type == ENTITY_TYPE::KRAB_BOSS) {
          registry.musics.insert(Entity(), MUSIC_ASSET_ID::INTRO_MUSIC);
        } else if (boss.type == ENTITY_TYPE::SHARKMAN) {
          registry.musics.insert(Entity(), MUSIC_ASSET_ID::LVL2_MUSIC);
        }
      }
    }
  }
}

void createDefaultHealthbar(RenderSystem* renderer, Entity& entity,
                            float health, vec2 healthScale, vec2 barScale,
                            vec2 bounding_box) {
  // Check if entity has a position component
  if (!registry.positions.has(entity)) {
    std::cerr << "Error: Entity does not have a position component"
              << std::endl;
    return;
  }

  // Create oxygen and background bar
  auto oxygenBar     = Entity();
  auto backgroundBar = Entity();

  // Store a reference to the potentially re-used mesh object
  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(oxygenBar, &mesh);
  registry.meshPtrs.emplace(backgroundBar, &mesh);

  // Get position of entity
  Position& entityPos = registry.positions.get(entity);

  // Setting initial positon values
  Position& position = registry.positions.emplace(oxygenBar);
  position.position =
      entityPos.position - vec2(0.f, entityPos.scale.y / 2 + ENEMY_O2_BAR_GAP);
  position.angle         = 0.f;
  position.scale         = healthScale * bounding_box;
  position.originalScale = healthScale * bounding_box;

  Position& backgroundPos = registry.positions.emplace(backgroundBar);
  backgroundPos.position  = position.position;
  backgroundPos.angle     = 0.f;
  backgroundPos.scale     = barScale * bounding_box;

  // Set health bar
  auto& entityOxygen         = registry.oxygen.emplace(entity);
  entityOxygen.capacity      = health;
  entityOxygen.level         = health;
  entityOxygen.rate          = 0.f;
  entityOxygen.isRendered    = false;
  entityOxygen.oxygenBar     = oxygenBar;
  entityOxygen.backgroundBar = backgroundBar;
}

void renderHealthBar(Oxygen& entity_oxygen) {
  registry.renderRequests.insert(
      entity_oxygen.oxygenBar,
      {TEXTURE_ASSET_ID::ENEMY_OXYGEN_BAR, EFFECT_ASSET_ID::TEXTURED,
       GEOMETRY_BUFFER_ID::SPRITE});

  registry.renderRequests.insert(
      entity_oxygen.backgroundBar,
      {TEXTURE_ASSET_ID::ENEMY_BACKGROUND_BAR, EFFECT_ASSET_ID::TEXTURED,
       GEOMETRY_BUFFER_ID::SPRITE});
}
