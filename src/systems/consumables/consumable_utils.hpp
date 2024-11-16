#pragma once

#include "tiny_ecs_registry.hpp"
#include "oxygen_system.hpp"

bool handle_consumable_collisions(Entity &player, Entity &consumable, RenderSystem* renderer);
