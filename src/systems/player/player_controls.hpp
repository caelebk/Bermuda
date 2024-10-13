#pragma once
#include "common.hpp"
#include "tiny_ecs.hpp"
#include "tiny_ecs_registry.hpp"

// Movement speed (To be balanced later)
#define SPEED_INC 2.0f

bool player_movement(int key, int action, int mod, Entity &player);

bool player_mouse(int key, int action, int mod, Entity &player, Entity &player_weapon, Entity &project_projectile);
