#pragma once

#include <vector>

#include "common.hpp"
#include "tiny_ecs_registry.hpp"
#include "random.hpp"

class AISystem {
  private:
  void do_wander_ai(float elapsed_ms);
  public:
  void step(float elapsed_ms);
};
