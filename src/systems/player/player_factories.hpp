#pragma once

#include "abilities.hpp"
#include "common.hpp"
#include "physics.hpp"
#include "player.hpp"
#include "render_system.hpp"
#include "tiny_ecs.hpp"

//////////////////////////////////////////////////////////////
// Player
//////////////////////////////////////////////////////////////
#define PLAYER_SCALE_FACTOR vec2(0.12f)
#define PLAYER_BOUNDING_BOX vec2(599.f, 540.f)  // vec2(PNG_width, PNG_height)
#define PLAYER_OXYGEN 1000.f
#define PLAYER_OXYGEN_SCALE_FACTOR vec2(0.7f)
#define PLAYER_OXYGEN_TANK_SCALE_FACTOR vec2(0.8f)
#define PLAYER_OXYGEN_BOUNDING_BOX vec2(100.f, 500.f)
#define PLAYER_OXYGEN_TANK_BOUNDING_BOX vec2(110.f, 570.f)
#define PLAYER_OXYGEN_RATE -3.f
#define PLAYER_DASH_COST -50.f
#define PLAYER_OXYGEN_DEPLETE_TIME_MS 1000.f  // to be balanced later
#define PLAYER_MASS 10
#define ARM_OFFSET vec2(20.f, -10.f)

Entity createPlayer(RenderSystem* renderer, vec2 pos);

// create oxygen tank for player
void createOxygenTank(RenderSystem* renderer, Entity& player, vec2 pos);

void createDashIndicator(RenderSystem* renderer, Entity& player, vec2 pos);

// create GUNS :eagle: :eagle: :eagle:
Entity createLoadedGun(RenderSystem* renderer, vec2 ammoPosition,
                       PROJECTILES projectile);

Entity createConsumableGun(RenderSystem* renderer, float oxy_cost,
                           PROJECTILES projectile);

//////////////////////////////////////////////////////////////
// Harpoon
//////////////////////////////////////////////////////////////
#define GUN_SCALE_FACTOR vec2(0.12f)
#define GUN_BOUNDING_BOX vec2(480.f, 100.f)
#define GUN_RELATIVE_POS_FROM_PLAYER vec2(23.f, 0.f)
#define HARPOON_GUN_OXYGEN_COST -25.f

#define HARPOON_SCALE_FACTOR vec2(0.1f)
#define HARPOON_BOUNDING_BOX vec2(340.f, 60.f)
#define HARPOON_RELATIVE_POS_FROM_GUN vec2(14.f, 0.f)

Entity loadHarpoon(RenderSystem* renderer, vec2 gunPosition);

//////////////////////////////////////////////////////////////
// Net
//////////////////////////////////////////////////////////////
#define NET_GUN_SCALE_FACTOR vec2(0.12f)
#define NET_GUN_BOUNDING_BOX vec2(451.f, 100.f)
#define NET_GUN_RELATIVE_POS_FROM_PLAYER vec2(21.f, 0.f)
#define NET_GUN_OXYGEN_COST -30.f

#define NET_SCALE_FACTOR vec2(0.05f)
#define NET_BOUNDING_BOX vec2(201.f, 290.f)
#define NET_RELATIVE_POS_FROM_GUN vec2(30.f, 0.f)
#define NET_OXYGEN_COST 0.f
#define NET_STUN_DURATION 5000.0f

Entity loadNet(RenderSystem* renderer);

//////////////////////////////////////////////////////////////
// Concussive
//////////////////////////////////////////////////////////////
#define CONCUSSIVE_GUN_SCALE_FACTOR vec2(0.12f)
#define CONCUSSIVE_GUN_BOUNDING_BOX vec2(460.f, 140.f)
#define CONCUSSIVE_GUN_RELATIVE_POS_FROM_PLAYER vec2(22.f, 0.f)
#define CONCUSSIVE_GUN_OXYGEN_COST -35.f

#define CONCUSSIVE_SCALE_FACTOR vec2(0.05f)
#define CONCUSSIVE_BOUNDING_BOX vec2(271.f, 470.f)
#define CONCUSSIVE_RELATIVE_POS_FROM_GUN vec2(30.f, 0.f)
#define CONCUSSIVE_OXYGEN_COST 0.f
#define CONCUSSIVE_KNOCKBACK_DURATION 500.f

Entity loadConcussive(RenderSystem* renderer);

//////////////////////////////////////////////////////////////
// Torpedo
//////////////////////////////////////////////////////////////
#define TORPEDO_GUN_SCALE_FACTOR vec2(0.12f)
#define TORPEDO_GUN_BOUNDING_BOX vec2(530.f, 140.f)
#define TORPEDO_GUN_RELATIVE_POS_FROM_PLAYER vec2(26.f, 0.f)
#define TORPEDO_GUN_OXYGEN_COST -50.f

#define TORPEDO_SCALE_FACTOR vec2(0.07f)
#define TORPEDO_BOUNDING_BOX vec2(420.f, 140.f)
#define TORPEDO_RELATIVE_POS_FROM_GUN vec2(20.f, 0.f)
#define TORPEDO_OXYGEN_COST -15.f
#define TORPEDO_DAMAGE_RADIUS 150;

Entity loadTorpedo(RenderSystem* renderer);

//////////////////////////////////////////////////////////////
// Shrimp
//////////////////////////////////////////////////////////////
#define SHRIMP_GUN_SCALE_FACTOR vec2(0.12f)
#define SHRIMP_GUN_BOUNDING_BOX vec2(509.f, 130.f)
#define SHRIMP_GUN_RELATIVE_POS_FROM_PLAYER vec2(25.f, 0.f)
#define SHRIMP_GUN_OXYGEN_COST -100.f

#define SHRIMP_SCALE_FACTOR vec2(0.10f)
#define SHRIMP_BOUNDING_BOX vec2(242.f, 130.f)
#define SHRIMP_RELATIVE_POS_FROM_GUN vec2(18.f, 0.f)
#define SHRIMP_OXYGEN_COST -100.f

Entity loadShrimp(RenderSystem* renderer);

//////////////////////////////////////////////////////////////
// Getters
//////////////////////////////////////////////////////////////
Entity& getPlayerWeapon();
Entity& getPlayerProjectile();

extern Entity player;
extern Entity harpoon;
extern Entity net;
extern Entity concussive;
extern Entity torpedo;
extern Entity shrimp;