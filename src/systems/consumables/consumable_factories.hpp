#pragma once

#include "common.hpp"
#include "render_system.hpp"
#include "tiny_ecs.hpp"
#include "tiny_ecs_registry.hpp"

//////////////////////////////////////////////////////////////
// Oxygen_Canister
//////////////////////////////////////////////////////////////
#define OXYGEN_CANISTER_QTY 100.0  // heals
#define OXYGEN_CANISTER_SCALE_FACTOR vec2(0.2f)
#define OXYGEN_CANISTER_BOUNDING_BOX \
  vec2(240.f, 112.f)  // vec2(PNG_width, PNG_height)

Entity createOxygenCanisterPos(RenderSystem* renderer, vec2 position, bool checkCollisions = true);
Entity respawnOxygenCanister(RenderSystem* renderer, EntityState es);

//////////////////////////////////////////////////////////////
// Nets
//////////////////////////////////////////////////////////////
#define NET_DROP_SCALE_FACTOR vec2(0.1f)
#define NET_DROP_BOUNDING_BOX \
  vec2(201.f, 290.f)  // vec2(PNG_width, PNG_height)

Entity createNetDropPos(RenderSystem* renderer, vec2 position,
                               bool checkCollisions = true);
Entity respawnNetDrop(RenderSystem* renderer, EntityState es);

//////////////////////////////////////////////////////////////
// CONCUSSIVE
//////////////////////////////////////////////////////////////
#define CONCUSSIVE_DROP_SCALE_FACTOR vec2(0.1f)
#define CONCUSSIVE_DROP_BOUNDING_BOX \
  vec2(271.f, 470.f)  // vec2(PNG_width, PNG_height)

Entity createConcussiveDropPos(RenderSystem* renderer, vec2 position,
                        bool checkCollisions = true);
Entity respawnConcussiveDrop(RenderSystem* renderer, EntityState es);

//////////////////////////////////////////////////////////////
// TORPEDO
//////////////////////////////////////////////////////////////
#define TORPEDO_DROP_SCALE_FACTOR vec2(0.1f)
#define TORPEDO_DROP_BOUNDING_BOX \
  vec2(420.f, 140.f)  // vec2(PNG_width, PNG_height)

Entity createTorpedoDropPos(RenderSystem* renderer, vec2 position,
                               bool checkCollisions = true);
Entity respawnTorpedoDrop(RenderSystem* renderer, EntityState es);

//////////////////////////////////////////////////////////////
// PISTOL SHRIMP (TO BE ADDED TO ANDY'S ENEMY)
//////////////////////////////////////////////////////////////
#define SHRIMP_DROP_SCALE_FACTOR vec2(0.2f)
#define SHRIMP_DROP_BOUNDING_BOX \
  vec2(242.f, 130.f)  // vec2(PNG_width, PNG_height)

Entity createShrimpDropPos(RenderSystem* renderer, vec2 position,
                               bool checkCollisions = true);
Entity respawnShrimpDrop(RenderSystem* renderer, EntityState es);

//////////////////////////////////////////////////////////////
// KEYS
//////////////////////////////////////////////////////////////
// TODO: Move these to item_factories.
// This extremely disgusting hack for unlockBossDoors doesn't actually render anything, it just unlocks every door in the room you're in
// and assumes you're in a boss room.
Entity unlockBossDoors(RenderSystem* renderer, vec2 position,
                               bool checkCollisions = true);

Entity createRedKeyPos(RenderSystem* renderer, vec2 position, bool checkCollisions = true);
Entity createRedKeyRespawnFn(RenderSystem *renderer, EntityState es);

Entity createBlueKeyPos(RenderSystem* renderer, vec2 position, bool checkCollisions = true);
Entity createBlueKeyRespawnFn(RenderSystem *renderer, EntityState es);

Entity createYellowKeyPos(RenderSystem* renderer, vec2 position, bool checkCollisions = true);
Entity createYellowKeyRespawnFn(RenderSystem *renderer, EntityState es);