#pragma once

#include "common.hpp"
#include "consumable_factories.hpp"
#include "render_system.hpp"
#include "tiny_ecs.hpp"
#include "tiny_ecs_registry.hpp"

#define DOOR_SPAWN_RADIUS 100.f

//////////////////////////////////////////////////////////////
// Geyser
//////////////////////////////////////////////////////////////
#define GEYSER_QTY 50.0       // heals
#define GEYSER_RATE_MS 1250.0  // heal rate
#define GEYSER_SCALE_FACTOR vec2(0.15f)
#define GEYSER_BOUNDING_BOX vec2(329.f, 344.f)  // vec2(PNG_width, PNG_height)

Entity createGeyserPos(RenderSystem* renderer, vec2 position,
                       bool checkCollisions = true);
Entity respawnGeyser(RenderSystem* renderer, EntityState es);

//////////////////////////////////////////////////////////////
// Crates
//////////////////////////////////////////////////////////////
#define CRATE_SCALE_FACTOR vec2(0.2f)
#define CRATE_BOUNDING_BOX vec2(297.f, 263.f)  // vec2(PNG_width, PNG_height)
#define CRATE_HEALTH 75.0                      // three shot to show workin
#define CRATE_HEALTH_SCALE vec2(1.4f)
#define CRATE_HEALTH_BAR_SCALE vec2(1.5f)
#define CRATE_HEALTH_BOUNDING_BOX vec2(50.f, 5.f)
#define CRATE_MASS 20

#define CRATE_DROP_0 createOxygenCanisterPos
#define CRATE_DROP_1 createBonesPos
#define CRATE_DROP_2 createJunkPos
#define CRATE_DROP_CHANCE_0 0.5

Entity createCratePos(RenderSystem* renderer, vec2 position,
                      bool checkCollisions = true);
Entity respawnCrate(RenderSystem* renderer, EntityState es);

//////////////////////////////////////////////////////////////
// ROCK
//////////////////////////////////////////////////////////////
#define ROCK_SCALE_FACTOR vec2(0.2f)
#define ROCK_BOUNDING_BOX vec2(167.f, 158.f)  // vec2(PNG_width, PNG_height)
#define ROCK_MASS 30.0

Entity createRockPos(RenderSystem* renderer, vec2 position,
                      bool checkCollisions = true);
Entity createCthulhuRocksPos(RenderSystem* renderer, vec2 position,
                              bool checkCollisions);
Entity respawnRock(RenderSystem* renderer, EntityState es);

#define METAL_CRATE_SCALE_FACTOR vec2(0.25f)
#define METAL_CRATE_BOUNDING_BOX vec2(256.f, 256.f)
#define METAL_CRATE_HEALTH 2000.0

Entity createMetalCratePos(RenderSystem* renderer, vec2 position,
                           bool checkCollisions = true);
Entity respawnMetalCrate(RenderSystem* renderer, EntityState es);
Entity createSharkmanCratesPos(RenderSystem* renderer, vec2 position,
                               bool checkCollisions = true);

#define PRESSURE_PLATE_SCALE_FACTOR vec2(0.6f)
#define PRESSURE_PLATE_BOUNDING_BOX vec2(100.f, 100.f)

Entity createPressurePlatePos(RenderSystem* renderer, vec2 position,
                           bool checkCollisions = true);
Entity respawnPressurePlate(RenderSystem* renderer, EntityState es);


//////////////////////////////////////////////////////////////
// Shells
//////////////////////////////////////////////////////////////
#define AMBIENT_MIN_SCALE 0.1
#define AMBIENT_MAX_SCALE 0.2
#define SHELL_BOUNDING_BOX vec2(200.f, 200.f)  // vec2(PNG_width, PNG_height)
Entity createShellPos(RenderSystem* renderer, vec2 position,
                           bool checkCollisions = true);
Entity respawnShell(RenderSystem* renderer, EntityState es);


//////////////////////////////////////////////////////////////
// Kelp
//////////////////////////////////////////////////////////////
#define KELP_BOUNDING_BOX vec2(200.f, 200.f)  // vec2(PNG_width, PNG_height)
Entity createKelpPos(RenderSystem* renderer, vec2 position,
                           bool checkCollisions = true);
Entity respawnKelp(RenderSystem* renderer, EntityState es);

//////////////////////////////////////////////////////////////
// Bones
//////////////////////////////////////////////////////////////
#define BONES_BOUNDING_BOX vec2(200.f, 200.f)  // vec2(PNG_width, PNG_height)
Entity createBonesPos(RenderSystem* renderer, vec2 position,
                           bool checkCollisions = true);
Entity respawnBones(RenderSystem* renderer, EntityState es);

//////////////////////////////////////////////////////////////
// Junk
//////////////////////////////////////////////////////////////
#define JUNK_BOUNDING_BOX vec2(200.f, 200.f)  // vec2(PNG_width, PNG_height)
Entity createJunkPos(RenderSystem* renderer, vec2 position,
                           bool checkCollisions = true);
Entity respawnJunk(RenderSystem* renderer, EntityState es);

//////////////////////////////////////////////////////////////
// Coral
//////////////////////////////////////////////////////////////
#define CORAL_BOUNDING_BOX vec2(200.f, 200.f)  // vec2(PNG_width, PNG_height)
Entity createCoralPos(RenderSystem* renderer, vec2 position,
                           bool checkCollisions = true);
Entity respawnCoral(RenderSystem* renderer, EntityState es);
