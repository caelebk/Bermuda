#pragma once
#include "common.hpp"

// All data relevant to the position of entities
struct Position {
  vec2 position = {0, 0};
  float angle = 0;
  vec2 scale = {10, 10};
  vec2 originalScale = {10, 10};
};

// All data relevant to the shape and motion of entities
struct Motion {
  vec2 acceleration = {0, 0};
  vec2 velocity = {0, 0};
};

struct Mass {
  int mass;
};

// Stucture to store collision information
struct Collision {
  // Note, the first object is stored in the ECS container.entities
  Entity other; // the second object involved in the collision
  Collision(Entity &other) { this->other = other; };
};

