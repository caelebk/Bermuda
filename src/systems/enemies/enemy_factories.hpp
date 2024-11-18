#pragma once

#include "common.hpp"
#include "physics.hpp"
#include "random.hpp"
#include "render_system.hpp"
#include "respawn.hpp"
#include "tiny_ecs.hpp"

#define ENEMY_O2_BAR_GAP 20.f
#define PLAYER_SPAWN_RADIUS 300.f

//////////////////////////////////////////////////////////////
// Group Spawning
//////////////////////////////////////////////////////////////
bool checkEnemySpawnCollisions(struct Position enemyPos);

//////////////////////////////////////////////////////////////
// JellyFish
//////////////////////////////////////////////////////////////
#define JELLY_DAMAGE -15.0
#define JELLY_ATK_SPD 1000.0
#define JELLY_STUN_MS 2000.0
#define JELLY_SCALE_FACTOR vec2(0.15f)
#define JELLY_BOUNDING_BOX vec2(266.f, 400.f)  // vec2(PNG_width, PNG_height)
#define JELLY_HEALTH 50.0                      // two shot to show workin
#define JELLY_HEALTH_SCALE vec2(1.4f)
#define JELLY_HEALTH_BAR_SCALE vec2(1.5f)
#define JELLY_HEALTH_BOUNDING_BOX vec2(50.f, 5.f)
#define JELLY_DROP_CHANCE_0 0.5f

Entity createJellyPos(RenderSystem* renderer, vec2 position,
                      bool checkCollisions = true);
Entity respawnJelly(RenderSystem* renderer, EntityState es);

//////////////////////////////////////////////////////////////
// Fish
//////////////////////////////////////////////////////////////
#define FISH_MS 20.0
#define FISH_DAMAGE -10.0
#define FISH_ATK_SPD 1000.0
#define FISH_SCALE_FACTOR vec2(0.09f)
#define FISH_BOUNDING_BOX vec2(512.f, 288.f)
#define FISH_HEALTH 1.0  // one shot
#define FISH_MIN_DIR_CD 1000
#define FISH_MAX_DIR_CD 8000
#define FISH_HEALTH_SCALE vec2(1.4f)
#define FISH_HEALTH_BAR_SCALE vec2(1.5f)
#define FISH_HEALTH_BOUNDING_BOX vec2(50.f, 5.f)

Entity createFishPos(RenderSystem* renderer, vec2 position,
                     bool checkCollisions = true);
Entity respawnFish(RenderSystem* renderer, EntityState es);

//////////////////////////////////////////////////////////////
// Sharks
//////////////////////////////////////////////////////////////

#define SHARK_MS 60.0
#define SHARK_DAMAGE -20.0
#define SHARK_ATK_SPD 1000.0
#define SHARK_MIN_SCALE 0.1
#define SHARK_MAX_SCALE 0.2
#define SHARK_BOUNDING_BOX vec2(765.f, 306.f)
#define SHARK_HEALTH 200.0  // four shot
#define SHARK_MIN_DIR_CD 1000
#define SHARK_MAX_DIR_CD 8000
#define SHARK_TRACKING_CD 1000.f
#define SHARK_SPOT_RADIUS 250.f
#define SHARK_LEASH_RADIUS 500.f
#define SHARK_TRACKING_ACCELERATION 10.f
#define SHARK_HEALTH_SCALE vec2(1.4f)
#define SHARK_HEALTH_BAR_SCALE vec2(1.5f)
#define SHARK_HEALTH_BOUNDING_BOX vec2(50.f, 5.f)
#define SHARK_DROP_CHANCE_0 0.15f

Entity createSharkPos(RenderSystem* renderer, vec2 position,
                      bool checkCollisions = true);
Entity respawnShark(RenderSystem* renderer, EntityState es);

// //////////////////////////////////////////////////////////////
// // Octopi
// //////////////////////////////////////////////////////////////

// #define OCTOPUS_OXYGEN 90.0
// #define OCTOPUS_OXYGEN_SCALE vec2(175.f, 4.f)

// int createOctopusHealthBar(RenderSystem *renderer, Entity enemy);

///////////////////////////////////////////////////////////////
// Krabs
///////////////////////////////////////////////////////////////
#define KRAB_MS 5.0
#define KRAB_DAMAGE -100.0
#define KRAB_ATK_SPD 1000.0
#define KRAB_MIN_SCALE 0.01
#define KRAB_MAX_SCALE 0.02
#define KRAB_BOUNDING_BOX vec2(3445.f, 2494.f)
#define KRAB_HEALTH 250.0  // three shot
#define KRAB_MIN_DIR_CD 1000
#define KRAB_MAX_DIR_CD 8000
#define KRAB_TRACKING_CD 1000.f
#define KRAB_SPOT_RADIUS 250.f
#define KRAB_LEASH_RADIUS 500.f
#define KRAB_TRACKING_ACCELERATION 10.f
#define KRAB_HEALTH_SCALE vec2(1.4f)
#define KRAB_HEALTH_BAR_SCALE vec2(1.5f)
#define KRAB_HEALTH_BOUNDING_BOX vec2(50.f, 5.f)
#define KRAB_DROP_CHANCE_0 0.3f

Entity createKrabPos(RenderSystem* renderer, vec2 position,
                     bool checkCollisions = true);
Entity respawnKrab(RenderSystem* renderer, EntityState es);

///////////////////////////////////////////////////////////////
// Sea Urchin
///////////////////////////////////////////////////////////////
#define URCHIN_MS 175.0
#define URCHIN_MIN_SCALE 0.4
#define URCHIN_MAX_SCALE 0.5
#define URCHIN_BOUNDING_BOX vec2(100.f, 100.f)
#define URCHIN_HEALTH 100  // two shot
#define URCHIN_MIN_DIR_CD 1000
#define URCHIN_MAX_DIR_CD 8000
#define URCHIN_HEALTH_SCALE vec2(1.4f)
#define URCHIN_HEALTH_BAR_SCALE vec2(1.5f)
#define URCHIN_HEALTH_BOUNDING_BOX vec2(50.f, 5.f)
#define URCHIN_FIRERATE 1500.f
#define URCHIN_DROP_CHANCE_0 0.75f

Entity createUrchinPos(RenderSystem* renderer, vec2 position,
                       bool checkCollisions = true);
Entity respawnUrchin(RenderSystem* renderer, EntityState es);

///////////////////////////////////////////////////////////////
// Sea Urchin Needle
///////////////////////////////////////////////////////////////
#define URCHIN_NEEDLE_MS 50.0
#define URCHIN_NEEDLE_SCALE_FACTOR vec2(0.5f)
#define URCHIN_NEEDLE_BOUNDING_BOX vec2(35.f, 18.f)
#define URCHIN_NEEDLE_DAMAGE -50.0
#define URCHIN_NEEDLE_TIMER 1500.f

Entity launchUrchinNeedle(RenderSystem* renderer, vec2 position, float angle);

///////////////////////////////////////////////////////////////
// Seahorse
///////////////////////////////////////////////////////////////
#define SEAHORSE_MS 30.0
#define SEAHORSE_MIN_SCALE 0.1
#define SEAHORSE_MAX_SCALE 0.15
#define SEAHORSE_BOUNDING_BOX vec2(255.f, 384.f)
#define SEAHORSE_HEALTH 100
#define SEAHORSE_MIN_DIR_CD 3000
#define SEAHORSE_MAX_DIR_CD 6000
#define SEAHORSE_HEALTH_SCALE vec2(1.4f)
#define SEAHORSE_HEALTH_BAR_SCALE vec2(1.5f)
#define SEAHORSE_HEALTH_BOUNDING_BOX vec2(50.f, 5.f)
#define SEAHORSE_FIRERATE 3000.f
// TODO: balance this for M4
#define SEAHORSE_DROP_CHANCE_0 0.2f

Entity createSeahorsePos(RenderSystem* renderer, vec2 position,
                         bool checkCollisions = true);
Entity respawnSeahorse(RenderSystem* renderer, EntityState es);

///////////////////////////////////////////////////////////////
// Seahorse Bullet
///////////////////////////////////////////////////////////////
#define SEAHORSE_BULLET_MS 300.0
#define SEAHORSE_BULLET_SCALE_FACTOR vec2(0.02f)
#define SEAHORSE_BULLET_BOUNDING_BOX vec2(1280.f, 280.f)
#define SEAHORSE_BULLET_DAMAGE -150.0
#define SEAHORSE_BULLET_TIMER 5000.f

Entity fireSeahorseBullet(RenderSystem* renderer, vec2 position,
                          vec2 direction);

///////////////////////////////////////////////////////////////
// Lobster
///////////////////////////////////////////////////////////////
#define LOBSTER_MS 30.0
#define LOBSTER_DAMAGE -100.0
#define LOBSTER_ATK_SPD 1000.0
#define LOBSTER_SCALE 0.2f
#define LOBSTER_BOUNDING_BOX vec2(595.f, 290.f)
#define LOBSTER_HEALTH 500.0  // six shot
#define LOBSTER_MIN_DIR_CD 1000
#define LOBSTER_MAX_DIR_CD 8000
#define LOBSTER_TRACKING_CD 1000.f
#define LOBSTER_SPOT_RADIUS 300.f
#define LOBSTER_LEASH_RADIUS 500.f
#define LOBSTER_TRACKING_ACCELERATION 10.f
#define LOBSTER_BLOCK_DURATION 1000.f
#define LOBSTER_BLOCK_MITIGATION 1.0f
#define LOBSTER_RAM_DURATION 500.f
#define LOBSTER_RAM_SPEED 200.f
#define LOBSTER_HEALTH_SCALE vec2(1.4f)
#define LOBSTER_HEALTH_BAR_SCALE vec2(1.5f)
#define LOBSTER_HEALTH_BOUNDING_BOX vec2(50.f, 5.f)

Entity createLobsterPos(RenderSystem* renderer, vec2 position,
                        bool checkCollisions = true);
Entity respawnLobster(RenderSystem* renderer, EntityState es);