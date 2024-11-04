#pragma once
#include <functional>
#include <vector>

// anything that is deadly to the player
struct Deadly {};

struct Boss {
  float curr_cd      = 0.f;
  float ai_cd  = 0.f;
  float max_proj_count;
  float curr_proj_count = 0.f;
  std::vector<std::function<void()>> ai;
};
