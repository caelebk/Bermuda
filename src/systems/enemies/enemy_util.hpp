#pragma once

#include <functional>
#include "render_system.hpp"
#include "tiny_ecs_registry.hpp"
#include "enemy_factories.hpp"
#include "common.hpp"

enum class EMOTE {
  NONE        = 0,
  EXCLAMATION = NONE + 1,
  QUESTION    = EXCLAMATION + 1,
};


bool update_attack(float elapsed_time_ms);

#define EMOTE_POS ENEMY_O2_BAR_GAP + 10.f
#define EMOTE_SCALE_FACTOR vec2(3.f)
#define EMOTE_BOUNDING_BOX vec2(10.f, 10.f)
void createEmote(RenderSystem *renderer, Entity &e, EMOTE emote);
void updateEmotePos(Entity& enemy);
