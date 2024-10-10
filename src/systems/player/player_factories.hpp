#pragma once

#include "common.hpp"
#include "render_system.hpp"
#include "tiny_ecs.hpp"

#include "abilities.hpp"
#include "physics.hpp"
#include "player.hpp"



//////////////////////////////////////////////////////////////
// Player
//////////////////////////////////////////////////////////////
#define PLAYER_SCALE_FACTOR vec2(0.7f)
#define PLAYER_BOUNDING_BOX vec2(42.f, 64.f) // vec2(PNG_width, PNG_height)
#define PLAYER_OXYGEN 1000.f
#define PLAYER_OXYGEN_BOUNDING_BOX vec2(20.f, 150.f) // TODO: update to actual values
#define PLAYER_OXYGEN_RATE 10.f
#define PLAYER_OXYGEN_DEPLETE_TIME_MS 3000.f // to be balanced later

Entity createPlayer(RenderSystem *renderer, vec2 pos, int projectile);

//////////////////////////////////////////////////////////////
// Gun
//////////////////////////////////////////////////////////////
#define GUN_SCALE_FACTOR vec2(0.7f)
#define GUN_BOUNDING_BOX vec2(60.f, 32.f)
#define GUN_RELATIVE_POS_FROM_PLAYER vec2(40.f, 0.f)
#define HARPOON_GUN_OXYGEN_COST 25.f

Entity createLoadedGun(RenderSystem *renderer, vec2 ammoPosition, int projectile);

//////////////////////////////////////////////////////////////
// Load Harpoon
//////////////////////////////////////////////////////////////
#define HARPOON_PROJECTILE 0
#define HARPOON_SCALE_FACTOR vec2(0.7f)
#define HARPOON_BOUNDING_BOX vec2(64.f, 26.f) 
#define HARPOON_RELATIVE_POS_FROM_GUN vec2(20.f, -2.f)

Entity loadHarpoon(RenderSystem *renderer, vec2 gunPosition);

//////////////////////////////////////////////////////////////
// Getters
//////////////////////////////////////////////////////////////
Entity &getPlayerWeapon(Entity& player);
Entity &getPlayerProjectile(Entity& player);
