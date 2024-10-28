#pragma once

#include "common.hpp"
#include "render_system.hpp"
#include "tiny_ecs.hpp"
#include "tiny_ecs_registry.hpp"

//////////////////////////////////////////////////////////////
// Geyser
//////////////////////////////////////////////////////////////
#define GEYSER_QTY 40.0       // heals
#define GEYSER_RATE_MS 500.0  // heals
#define GEYSER_SCALE_FACTOR vec2(0.05f)
#define GEYSER_BOUNDING_BOX vec2(512.f, 512.f)  // vec2(PNG_width, PNG_height)

Entity createGeyserPos(RenderSystem* renderer, vec2 position);
