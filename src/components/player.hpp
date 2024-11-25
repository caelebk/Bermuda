#pragma once

// Player HUD component
struct PlayerHUD {};

// Enum for weapon types
enum class PROJECTILES {
  HARPOON    = 0,
  NET        = HARPOON + 1,
  CONCUSSIVE = NET + 1,
  TORPEDO    = CONCUSSIVE + 1,
  SHRIMP     = TORPEDO + 1,
  PROJ_COUNT = SHRIMP + 1
};

// Projectile component
struct PlayerProjectile {
  bool is_loaded = false;  // flags whether to follow a weapon's position or act
                           // as a projectile
  bool        is_flipped;
  PROJECTILES type;
};

// Weapon component
struct PlayerWeapon {
  Entity projectile;
};

// Consumable inventory
struct Inventory {
  unsigned int nets       = 0;
  unsigned int concussors = 0;
  unsigned int torpedos   = 0;
  unsigned int shrimp     = 0;
  bool         redKey     = false;
  bool         blueKey    = false;
  bool         yellowKey  = false;
};

// Enum for inventory types
enum class INVENTORY {
  HARPOON    = 0,
  NET        = HARPOON + 1,
  CONCUSSIVE = NET + 1,
  TORPEDO    = CONCUSSIVE + 1,
  SHRIMP     = TORPEDO + 1,
  RED_KEY    = SHRIMP + 1,
  BLUE_KEY   = RED_KEY + 1,
  YELLOW_KEY = BLUE_KEY + 1,
  PROJ_COUNT = YELLOW_KEY + 1
};

// Inventory Counter
struct InventoryCounter {
  INVENTORY inventoryType;
};

// Communication
struct Communication {
  std::string line;
};

// Player component
struct Player {
  Entity weapon;
  Entity collisionMesh;
  Entity dashIndicator;
  // Controls
  bool upHeld            = false;
  bool downHeld          = false;
  bool leftHeld          = false;
  bool rightHeld         = false;
  bool gliding           = false;
  bool dashing           = false;
  int  dashTimer         = 0;
  int  dashCooldownTimer = 0;
};

struct PlayerCollisionMesh {};