#pragma once
#include <functional>
#include <vector>

#include "render_system.hpp"
#include "respawn.hpp"

enum class RangedEnemies {
  URCHIN,
  SEAHORSE,
  SIREN,
  CTHULHU_TENTACLE,
  CTHULHU_FIREBALL,
  CTHULHU_CANISTER,
  CTHULHU_SHOCKWAVE,
  CTHULHU_RAGE_PROJ
};

// anything that is deadly to the player
struct Deadly {
  ENTITY_TYPE type;
};

struct EnemyProjectile {
  ENTITY_TYPE type;
  bool        has_timer = false;
  float       timer;
};

struct EnemySupport {
  Entity user;
  bool   ignores_user;
};

struct Boss {
  ENTITY_TYPE                        type;
  float                              curr_cd = 0.f;
  float                              ai_cd   = 0.f;
  float                              max_proj_count;
  float                              curr_proj_count = 0.f;
  bool                               is_angry        = false;
  bool                               in_transition   = false;
  std::vector<std::function<void()>> ai;
};

struct Lobster {
  float original_speed;
  float block_duration;
  float block_timer = 0;
  float block_mitigation;
  float ram_duration;
  float ram_timer = 0;
  float ram_speed;
};
