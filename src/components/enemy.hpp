#pragma once
// anything that is deadly to the player
struct Deadly {};

struct DamageOnTouch {
  float amount = 0.0;
};

// prevents entities from constantly doing damage
struct AttackCD {
  float attack_spd = 0.f;
  float attack_cd = 0.f;
};
