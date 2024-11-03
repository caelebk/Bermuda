#pragma once

#include "common.hpp"
#include "render_system.hpp"
#include "tiny_ecs.hpp"
#include "tiny_ecs_registry.hpp"

//////////////////////////////////////////////////////////////
// Oxygen_Canister
//////////////////////////////////////////////////////////////
#define OXYGEN_CANISTER_QTY 100.0  // heals
#define OXYGEN_CANISTER_SCALE_FACTOR vec2(0.08f)
#define OXYGEN_CANISTER_BOUNDING_BOX \
  vec2(512.f, 512.f)  // vec2(PNG_width, PNG_height)

Entity createOxygenCanisterPos(RenderSystem* renderer, vec2 position);
