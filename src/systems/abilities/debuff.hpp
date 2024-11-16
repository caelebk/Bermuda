#pragma once
#include "common.hpp"

#define STUN_MOVEMENT_THRESHOLD_MS 1000
#define KNOCKBACK_MULTIPLIER 0.75f

bool handle_stun(Entity enemy, Entity player);

bool handle_knockback(Entity enemy, Entity player);

bool debuff_entity_can_move(Entity &entity);

bool debuff_entity_knockedback(Entity &entity);

bool update_debuffs(float elapsed_ms_since_last_update);

bool handle_debuffs(Entity player, Entity enemy);

extern Entity player_projectile;
extern Entity player;