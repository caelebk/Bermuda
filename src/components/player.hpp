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
};