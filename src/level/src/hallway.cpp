#include <stdio.h>

#include "hallway.hpp"

#include "tiny_ecs_registry.hpp"

Hallway::Hallway() {};

HallwayBuilder::HallwayBuilder(): SpaceBuilder<Hallway>() {
    new_entity(Hallway());
};