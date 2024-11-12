#pragma once

#include "common.hpp"
#include "render_system.hpp"
#include "tiny_ecs.hpp"
#include "tiny_ecs_registry.hpp"
#include "consumable_factories.hpp"

//////////////////////////////////////////////////////////////
// Geyser
//////////////////////////////////////////////////////////////
#define GEYSER_QTY 40.0       // heals
#define GEYSER_RATE_MS 500.0  // heals
#define GEYSER_SCALE_FACTOR vec2(0.15f)
#define GEYSER_BOUNDING_BOX vec2(329.f, 344.f) // vec2(PNG_width, PNG_height)

Entity createGeyserPos(RenderSystem *renderer, vec2 position);

//////////////////////////////////////////////////////////////
// Crate
//////////////////////////////////////////////////////////////
#define CRATE_SCALE_FACTOR vec2(0.05f)
#define CRATE_BOUNDING_BOX vec2(297.f, 263.f) // vec2(PNG_width, PNG_height)
#define CRATE_HEALTH 75.0 // three shot to show workin
#define CRATE_HEALTH_SCALE vec2(1.4f)
#define CRATE_HEALTH_BAR_SCALE vec2(1.5f)
#define CRATE_HEALTH_BOUNDING_BOX vec2(50.f, 5.f)

#define CRATE_DROP_0 createOxygenCanisterPos
#define CRATE_DROP_CHANCE_0 0.5


Entity createCratePos(RenderSystem *renderer, vec2 position);

Entity createGeyserPos(RenderSystem* renderer, vec2 position);
