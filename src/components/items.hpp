#pragma once
#include "common.hpp"
#include "player.hpp"
#include "render_system.hpp"
#include "respawn.hpp"
#include "entity_type.hpp"

struct Key {};

// anything the player can pick up, temp
struct Consumable {
  ENTITY_TYPE type;
};

// anything the player can pick up, perm
struct Item {
  Objective item;
  ENTITY_TYPE type;
  std::function<Entity(RenderSystem *renderer, EntityState es)> respawnFn;
};

// drops when entity is dead
struct Drop {
  std::function<Entity(RenderSystem *r, vec2 p, bool b)> dropFn;
};

struct WeaponDrop {
  INVENTORY type;
};
