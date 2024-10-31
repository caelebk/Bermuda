#pragma once

#include <vector>

#include "common.hpp"
#include "physics.hpp"
#include "render_system.hpp"
#include "tiny_ecs_registry.hpp"
#include "random.hpp"

class AISystem {
  private:
  RenderSystem * renderer;
  void do_wander_ai(float elapsed_ms);
  void do_wander_ai_line(float elapsed_ms);
  void do_wander_ai_square(float elapsed_ms);
  void do_track_player(float elapsed_ms);
  bool can_see_player(Position &pos, Position &player_pos);
  bool in_range_of_player(Position &pos, Position &player_pos, float range);
  public:
  void step(float elapsed_ms);
  void init(RenderSystem* renderer_arg);
};
