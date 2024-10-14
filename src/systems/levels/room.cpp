#include "room.hpp"

#include <stdio.h>

#include <chrono>
#include <random>
#include <thread>

#include "tiny_ecs_registry.hpp"

Room::Room() : Entity(){};

RoomBuilder::RoomBuilder() : SpaceBuilder<Room>() {
  new_entity(Room());
};