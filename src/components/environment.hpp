#pragma once

#include "common.hpp"
#include <limits>

/*
  "To instantiate a global variable; this path poisons the mind and corrupts the code.
   To disguise it as a Component; this path is that to harmony."
   -Confucius"
*/

// An enumeration for programatically defining directions.
// Not strongly-typed (enum class) for now.
enum Direction {
  NORTH = 0,
  EAST = NORTH + 1,
  SOUTH = EAST + 1,
  WEST = SOUTH + 1
};

struct SpaceBoundingBox {
    float minimum_x = std::numeric_limits<float>::max();
    float maximum_x = std::numeric_limits<float>::min();
    float minimum_y = std::numeric_limits<float>::max();
    float maximum_y = std::numeric_limits<float>::min();
};

// TODO:
// This is only used internally for SpaceBuilder, so probably just remove it from the ECS entirely.
// Also the name sucks.
struct Vector {
  vec2 start;
  vec2 end;

  Vector(vec2 start, vec2 end) : start(start), end(end) {};
};

struct Space {
  std::vector<Entity> boundaries;
  std::vector<Entity> walls;
  std::vector<Entity> doors;
};

struct DoorConnection {
  Direction direction;
  std::string room_id;
  Entity exit_door;
};

struct ActiveWall {};

struct ActiveDoor {};

struct Interactable {};
