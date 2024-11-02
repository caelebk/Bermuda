#pragma once
#include "common.hpp"
#include "render_system.hpp"

// anything the player can pick up, temp
struct Consumable {};

// anything the player can pick up, perm
struct Item {};

// drops when entity is dead
struct Drop {
  std::function<Entity(RenderSystem *r, vec2 p)> dropFn;
};
