#pragma once
#include <functional>
#include <vector>
#include "render_system.hpp"
#include "respawn.hpp"

enum class BossType {
  KRAB_BOSS,
  SHARKMAN
};

enum class RangedEnemies {
  URCHIN
};

// anything that is deadly to the player
struct Deadly {
  std::function<void(RenderSystem *renderer, EntityState es)> respawnFn;
};

struct EnemyProjectile {
  bool has_timer = false;
  float timer;
};

struct Boss {
  enum BossType type;
  float curr_cd      = 0.f;
  float ai_cd  = 0.f;
  float max_proj_count;
  float curr_proj_count = 0.f;
  bool is_angry = false;
  std::vector<std::function<void()>> ai;
};
