#include "enemy_util.hpp"

#include <cstdio>
#include <glm/common.hpp>

#include "components.hpp"
#include "enemy.hpp"
#include "misc.hpp"
#include "oxygen.hpp"
#include "render_system.hpp"
#include "tiny_ecs_registry.hpp"

/**
 * @brief updates all enemies
 *
 * @return
 */
bool update_attack(float elapsed_time_ms) {
  for (Entity& e : registry.modifyOxygenCd.entities) {
    ModifyOxygenCD& attackCd = registry.modifyOxygenCd.get(e);
    if (attackCd.curr_cd == 0.f) {
      continue;
    }
    attackCd.curr_cd         = max(attackCd.curr_cd - elapsed_time_ms, 0.f);
  }

  return true;
}

void createEmote(RenderSystem* renderer, Entity& e, EMOTE emote) {
  if (!registry.positions.has(e)) {
    return;
  }

  if (registry.emoting.has(e)) {
    Emoting& temp = registry.emoting.get(e);
    registry.remove_all_components_of(temp.child);
    registry.emoting.remove(e);
    printf("Removing Emote!\n");
  }

  if (emote == EMOTE::NONE) {
    return;
  }

  printf("Creating Emote!\n");

  Position&        entityPos  = registry.positions.get(e);
  Emoting&         curr_emote = registry.emoting.emplace(e);
  Entity           child      = Entity();
  TEXTURE_ASSET_ID emote_texture;
  curr_emote.child = child;

  switch (emote) {
    case EMOTE::EXCLAMATION:
      emote_texture = TEXTURE_ASSET_ID::EMOTE_EXCLAMATION;
      break;
    case EMOTE::QUESTION:
      emote_texture = TEXTURE_ASSET_ID::EMOTE_QUESTION;
      break;
  }

  Position& position = registry.positions.emplace(child);
  // Setting initial positon values
  position.position =
      entityPos.position -
      vec2(0.f, entityPos.scale.y / 2 +
                    EMOTE_POS);  // TODO: guesstimate on where the HP
                                 // should be, update to proper position
  position.angle         = 0.f;
  position.scale         = EMOTE_SCALE_FACTOR * EMOTE_BOUNDING_BOX;
  position.originalScale = EMOTE_SCALE_FACTOR * EMOTE_BOUNDING_BOX;

  Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
  registry.meshPtrs.emplace(child, &mesh);
  registry.renderRequests.insert(
      child,
      {emote_texture, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});
}

/**
 * @brief update the position values of an entity's health bar depending on
 * entity position
 */
void updateEmotePos(Entity& enemy) {
  Position& enemyPos = registry.positions.get(enemy);
  Emoting&  emote =
      registry.emoting.get(enemy);  // TODO: ensure no unexpected behaviour once
                                    // collision handling is fully implemented

  Position& emotePos = registry.positions.get(emote.child);
  emotePos.position =
      enemyPos.position - vec2(0.f, enemyPos.scale.y / 2 + EMOTE_POS);
}
