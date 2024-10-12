#pragma once

#include "common.hpp"
#include "render_system.hpp"
#include "tiny_ecs.hpp"
#include "tiny_ecs_registry.hpp"

//////////////////////////////////////////////////////////////
// Oxygen_Tank
//////////////////////////////////////////////////////////////
#define OXYGEN_TANK_QTY -100.0 // heals
#define OXYGEN_TANK_SCALE_FACTOR vec2(0.05f)
#define OXYGEN_TANK_BOUNDING_BOX vec2(512.f, 512.f) // vec2(PNG_width, PNG_height)

Entity createOxygenTankPos(RenderSystem *renderer, vec2 position);

