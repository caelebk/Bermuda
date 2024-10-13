#pragma once

#include "common.hpp"
#include "render_system.hpp"
#include "tiny_ecs.hpp"

#define ENEMY_O2_BAR_GAP 20.f

//////////////////////////////////////////////////////////////
// JellyFish
//////////////////////////////////////////////////////////////
#define JELLY_DAMAGE 15.0
#define JELLY_ATK_SPD 1000.0
#define JELLY_STUN_MS 2000.0
#define JELLY_SCALE_FACTOR vec2(1.4f)
#define JELLY_BOUNDING_BOX vec2(24.f, 24.f) // vec2(PNG_width, PNG_height)
#define JELLY_HEALTH 50.0 // two shot to show workin
#define JELLY_HEALTH_SCALE vec2(1.4f)
#define JELLY_HEALTH_BAR_SCALE vec2(1.5f)
#define JELLY_HEALTH_BOUNDING_BOX vec2(50.f, 5.f)

Entity createJellyPos(RenderSystem *renderer, vec2 position);
void createJellyHealthBar(RenderSystem *renderer, Entity &enemy);

//////////////////////////////////////////////////////////////
// Fish
//////////////////////////////////////////////////////////////
#define FISH_MS 20.0
#define FISH_DAMAGE 5.0
#define FISH_ATK_SPD 1000.0
#define FISH_SCALE_FACTOR vec2(0.4f)
#define FISH_BOUNDING_BOX vec2(108.f, 77.f)
#define FISH_HEALTH 1.0 // one shot
#define FISH_HEALTH_SCALE vec2(1.4f)
#define FISH_HEALTH_BAR_SCALE vec2(1.5f)
#define FISH_HEALTH_BOUNDING_BOX vec2(50.f, 5.f)

Entity createFishPos(RenderSystem *renderer, vec2 position);
void createFishHealthBar(RenderSystem *renderer, Entity &enemy);

//////////////////////////////////////////////////////////////
// Sharks
//////////////////////////////////////////////////////////////

// #define SHARK_OXYGEN 75.0
// #define SHARK_OXYGEN_SCALE vec2(250.f, 4.f)

// int createSharkHealthBar(RenderSystem *renderer, Entity enemy);

// //////////////////////////////////////////////////////////////
// // Octopi
// //////////////////////////////////////////////////////////////

// #define OCTOPUS_OXYGEN 90.0
// #define OCTOPUS_OXYGEN_SCALE vec2(175.f, 4.f)

// int createOctopusHealthBar(RenderSystem *renderer, Entity enemy);

// //////////////////////////////////////////////////////////////
// // Krabs
// //////////////////////////////////////////////////////////////

// #define KRAB_OXYGEN 30.0
// #define KRAB_OXYGEN_SCALE vec2(125.f, 4.f)

// int createKrabHealthBar(RenderSystem *renderer, Entity enemy);

// //////////////////////////////////////////////////////////////
// // Sea mine
// //////////////////////////////////////////////////////////////

// #define SEA_MINE_OXYGEN 25.0
// #define SEA_MINE_OXYGEN_SCALE vec2(80.f, 4.f)

// int createSeaMineHealthBar(RenderSystem *renderer, Entity enemy);

// //////////////////////////////////////////////////////////////
// // Merpeople
// //////////////////////////////////////////////////////////////

// #define MERPERSON_OXYGEN 120.0
// #define MERPERSON_OXYGEN_SCALE vec2(100.f, 4.f)

// int createMerpersonHealthBar(RenderSystem *renderer, Entity enemy);

// //////////////////////////////////////////////////////////////
// // Void Tentacles
// //////////////////////////////////////////////////////////////

// #define TENTACLE_OXYGEN 20.0
// #define TENTACLE_OXYGEN_SCALE vec2(50.f, 4.f)

// int createTentacleHealthBar(RenderSystem *renderer, Entity enemy);

// //////////////////////////////////////////////////////////////
// // Serpents
// //////////////////////////////////////////////////////////////

// #define SERPENT_OXYGEN 50.0
// #define SERPENT_OXYGEN_SCALE vec2(200.f, 4.f)

// int createSerpentHealthBar(RenderSystem *renderer, Entity enemy);
