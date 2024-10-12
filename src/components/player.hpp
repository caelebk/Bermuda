#pragma once
// Projectile component
struct PlayerProjectile {
  bool is_loaded = false; // flags whether to follow a weapon's position or act as a projectile
};

// Weapon component
struct PlayerWeapon {
  Entity projectile;
};

// Player component
struct Player {
  Entity weapon;

  // Controls
  bool upHeld = false;
  bool downHeld = false;
  bool leftHeld = false;
  bool rightHeld = false;
  bool dashing = false;
};