#pragma once
// A timer that will be associated to the dying
struct DeathTimer {
  float counter_ms = 0;
};

// Will wander aimlessly
struct Wander {
  float active_dir_cd = 0;
  float change_dir_cd = 0;
};
