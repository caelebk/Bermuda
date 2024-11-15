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
#define GEYSER_QTY 40.0       // heals
#define GEYSER_RATE_MS 500.0  // heals
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
#define CRATE_DROP_CHANCE_0 0.5

Entity createCratePos(RenderSystem* renderer, vec2 position,
                      bool checkCollisions = true);
Entity respawnCrate(RenderSystem* renderer, EntityState es);

#define METAL_CRATE_SCALE_FACTOR vec2(0.25f)
#define METAL_CRATE_BOUNDING_BOX vec2(256.f, 256.f)
#define METAL_CRATE_HEALTH 2000.0

Entity createMetalCratePos(RenderSystem* renderer, vec2 position,
                           bool checkCollisions = true);
Entity respawnMetalCrate(RenderSystem* renderer, EntityState es);
Entity createSharkmanCratesPos(RenderSystem* renderer, vec2 position,
                               bool checkCollisions = true);
