#pragma once
#include "common.hpp"

#define STUN_MOVEMENT_THRESHOLD_MS 1000
bool handle_stun(Entity enemy, Entity player);

bool debuff_entity_can_move(Entity &entity);

bool update_debuffs(float elapsed_ms_since_last_update);

bool handle_debuffs(Entity enemy, Entity player);
