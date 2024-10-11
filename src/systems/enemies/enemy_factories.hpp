#include "common.hpp"
#include "render_system.hpp"
#include "tiny_ecs.hpp"
#include <functional>

#define ENEMY_O2_BAR_GAP 20.f

//////////////////////////////////////////////////////////////
// JellyFish
//////////////////////////////////////////////////////////////
#define JELLY_DAMAGE 5.0
#define JELLY_STUN_MS 2000.0
#define JELLY_SCALE_FACTOR vec2(1.5f)
#define JELLY_BOUNDING_BOX vec2(24.f, 24.f) // vec2(PNG_width, PNG_height)
#define JELLY_OXYGEN 10.0
#define JELLY_OXYGEN_SCALE vec2(24.f, 4.f)

Entity createJellyRoom(RenderSystem *renderer, vec2 (*randPos)(void));
Entity createJellyPos(RenderSystem *renderer, vec2 position);
int createJellyHealthBar(RenderSystem *renderer, Entity enemy);

//////////////////////////////////////////////////////////////
// Fish
//////////////////////////////////////////////////////////////
#define FISH_MS 1.0
#define FISH_DAMAGE 5.0
#define FISH_SCALE_FACTOR vec2(0.5f)
#define FISH_BOUNDING_BOX vec2(108.f, 77.f)
#define FISH_OXYGEN 1.0 // one shot
#define FISH_OXYGEN_SCALE vec2(108.f, 4.f)

Entity createFishRoom(RenderSystem *renderer, vec2 (*randPos)(void));
Entity createFishPos(RenderSystem *renderer, vec2 position);
int createFishHealthBar(RenderSystem *renderer, Entity enemy);

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