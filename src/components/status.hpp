struct LowOxygen {};

struct Attacked {
  float timer;
};

struct Stunned {
  vec2 original_velocity;
  float duration;
};

struct KnockedBack {
  vec2 original_velocity;
  vec2 knocked_velocity;
  float duration;
  Entity knockback_proj;
};