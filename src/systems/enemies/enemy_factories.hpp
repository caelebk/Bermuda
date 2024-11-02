#pragma once

#include "common.hpp"
#include "random.hpp"
#include "render_system.hpp"
#include "tiny_ecs.hpp"

#define ENEMY_O2_BAR_GAP 20.f

//////////////////////////////////////////////////////////////
// JellyFish
//////////////////////////////////////////////////////////////
#define JELLY_DAMAGE -15.0
#define JELLY_ATK_SPD 1000.0
#define JELLY_STUN_MS 2000.0
#define JELLY_SCALE_FACTOR vec2(1.4f)
#define JELLY_BOUNDING_BOX vec2(24.f, 24.f)  // vec2(PNG_width, PNG_height)
#define JELLY_HEALTH 50.0                    // two shot to show workin
#define JELLY_HEALTH_SCALE vec2(1.4f)
#define JELLY_HEALTH_BAR_SCALE vec2(1.5f)
#define JELLY_HEALTH_BOUNDING_BOX vec2(50.f, 5.f)

Entity createJellyPos(RenderSystem* renderer, vec2 position);

//////////////////////////////////////////////////////////////
// Fish
//////////////////////////////////////////////////////////////
#define FISH_MS 20.0
#define FISH_DAMAGE -10.0
#define FISH_ATK_SPD 1000.0
#define FISH_SCALE_FACTOR vec2(0.09f)
#define FISH_BOUNDING_BOX vec2(512.f, 288.f)
#define FISH_HEALTH 1.0  // one shot
#define FISH_MIN_DIR_CD 1000 // random direction change cooldown so it looks more natural
#define FISH_MAX_DIR_CD 8000
#define FISH_HEALTH_SCALE vec2(1.4f)
#define FISH_HEALTH_BAR_SCALE vec2(1.5f)
#define FISH_HEALTH_BOUNDING_BOX vec2(50.f, 5.f)

Entity createFishPos(RenderSystem* renderer, vec2 position);

//////////////////////////////////////////////////////////////
// Sharks
//////////////////////////////////////////////////////////////

#define SHARK_MS 30.0
#define SHARK_DAMAGE -20.0
#define SHARK_ATK_SPD 1000.0
#define SHARK_MIN_SCALE 2.0
#define SHARK_MAX_SCALE 3.0
#define SHARK_BOUNDING_BOX vec2(32.f, 19.f)
#define SHARK_HEALTH 75.0  // three shot
#define SHARK_MIN_DIR_CD 1000 // random direction change cooldown so it looks more natural
#define SHARK_MAX_DIR_CD 8000
#define SHARK_TRACKING_CD 1000.f
#define SHARK_SPOT_RADIUS 250.f
#define SHARK_LEASH_RADIUS 500.f
#define SHARK_TRACKING_ACCELERATION 10.f
#define SHARK_HEALTH_SCALE vec2(1.4f)
#define SHARK_HEALTH_BAR_SCALE vec2(1.5f)
#define SHARK_HEALTH_BOUNDING_BOX vec2(50.f, 5.f)


Entity createSharkPos(RenderSystem* renderer, vec2 position);

// //////////////////////////////////////////////////////////////
// // Octopi
// //////////////////////////////////////////////////////////////

// #define OCTOPUS_OXYGEN 90.0
// #define OCTOPUS_OXYGEN_SCALE vec2(175.f, 4.f)

// int createOctopusHealthBar(RenderSystem *renderer, Entity enemy);

///////////////////////////////////////////////////////////////
// Krabs
///////////////////////////////////////////////////////////////
#define KRAB_MS 10.0
#define KRAB_DAMAGE -100.0
#define KRAB_ATK_SPD 1000.0
#define KRAB_MIN_SCALE 2.0
#define KRAB_MAX_SCALE 3.0
#define KRAB_BOUNDING_BOX vec2(18.f, 14.f)
#define KRAB_HEALTH 250.0  // three shot
#define KRAB_MIN_DIR_CD 1000 // random direction change cooldown so it looks more natural
#define KRAB_MAX_DIR_CD 8000
#define KRAB_TRACKING_CD 1000.f
#define KRAB_SPOT_RADIUS 250.f
#define KRAB_LEASH_RADIUS 500.f
#define KRAB_TRACKING_ACCELERATION 10.f
#define KRAB_HEALTH_SCALE vec2(1.4f)
#define KRAB_HEALTH_BAR_SCALE vec2(1.5f)
#define KRAB_HEALTH_BOUNDING_BOX vec2(50.f, 5.f)


Entity createKrabPos(RenderSystem* renderer, vec2 position);


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
