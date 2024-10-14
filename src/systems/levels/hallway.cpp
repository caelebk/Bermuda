#include "hallway.hpp"

#include <stdio.h>

#include "tiny_ecs_registry.hpp"

Hallway::Hallway(){};

HallwayBuilder::HallwayBuilder() : SpaceBuilder<Hallway>() {
  new_entity(Hallway());
};