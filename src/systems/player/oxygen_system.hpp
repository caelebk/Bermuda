#pragma once

#include "common.hpp"
#include "render_system.hpp"
#include "tiny_ecs.hpp"
#include "oxygen.hpp"

// create oxygen tank for player
Entity createOxygenTank(RenderSystem *renderer, vec2 pos);

// get oxygen level
float getOxygenLevel(Entity entity);

// set oxygen capacity
bool setOxygenCapacity(Entity entity, float capacity);

// deplete oxygen (passive consumption over time)
void depleteOxygen(Entity entity);

// consume oxygen (using items/abilities)
void consumeOxygen(Entity entity, Entity affector);

// refill oxygen 
void refillOxygen(Entity entity, Entity affector);