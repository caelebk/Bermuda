#pragma once
#include "common.hpp"
#include "tiny_ecs.hpp"
#include "tiny_ecs_registry.hpp"

// Movement speed (To be balanced later)
#define SPEED_INC 2.0f

bool player_movement(int key, int action, int mod);

bool player_mouse(int key, int action, int mod, Entity& default_wep);

void swapWeps(Entity swapped, Entity swapper, int projectile);

void handleWeaponSwapping(int key);

extern Entity player;
extern Entity player_weapon;
extern Entity player_projectile;
extern Entity harpoon;
extern Entity net;
extern int    wep_type;