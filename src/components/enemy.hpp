#pragma once
// anything that is deadly to the player
struct Deadly {};

// prevents entities from constantly doing damage
struct AttackCD {
  float attack_spd = 0.f;
  float attack_cd  = 0.f;
};
