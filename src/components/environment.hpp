#pragma once

#include "common.hpp"
#include <limits>

struct SpaceBoundingBox {
    float minimum_x = std::numeric_limits<float>::max();
    float maximum_x = std::numeric_limits<float>::min();
    float minimum_y = std::numeric_limits<float>::max();
    float maximum_y = std::numeric_limits<float>::min();
};

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

struct Adjacency {
  std::vector<Entity> neighbours;
};

struct ActiveWall {};

struct Interactable {};
