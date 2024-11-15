#pragma once
#include "common.hpp"
#include "tiny_ecs.hpp"
#include "tiny_ecs_registry.hpp"

// Movement speed (To be balanced later)
#define SPEED_INC 6.0f

bool player_movement(int key, int action, int mod);

bool player_mouse(RenderSystem* renderer, int key, int action, int mod, Entity& default_wep,
                  Entity& default_gun);

bool updateInventory(RenderSystem* renderer, PROJECTILES type);

void swapWeps(Entity swapped, Entity swapper, PROJECTILES projectile);

void handleGunSwap(Entity swapped, Entity swapper, PROJECTILES projectile);

void handleWeaponSwapping(int key);

// Helper for handleWeaponSwapping and player_mouse
void doWeaponSwap(Entity swapper_proj, Entity swapper_wep,
                  PROJECTILES projectile);

bool destroyGunOrProjectile(Entity entity);

Entity createPauseMenu(RenderSystem* renderer);

extern Entity      player;
extern Entity      player_weapon;
extern Entity      player_projectile;
extern Entity      harpoon;
extern Entity      net;
extern Entity      concussive;
extern Entity      torpedo;
extern Entity      shrimp;
extern PROJECTILES wep_type;

extern Entity harpoon_gun;
extern Entity net_gun;
extern Entity concussive_gun;
extern Entity torpedo_gun;
extern Entity shrimp_gun;