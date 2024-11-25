// Global definition of all entities
#pragma once

enum class ENTITY_TYPE {
  PLAYER          = 1,
  JELLY           = PLAYER + 1,
  FISH            = JELLY + 1,
  SHARK           = FISH + 1,
  KRAB            = SHARK + 1,
  URCHIN          = KRAB + 1,
  SEAHORSE        = URCHIN + 1,
  LOBSTER         = SEAHORSE + 1,
  BREAKABLE_CRATE = LOBSTER + 1,
  METAL_CRATE     = BREAKABLE_CRATE + 1,
  ROCK            = METAL_CRATE + 1,
  PRESSURE_PLATE  = ROCK + 1,
  OXYGEN_CANISTER = PRESSURE_PLATE + 1,
  GEYSER          = OXYGEN_CANISTER + 1,
  CONCUSSIVE      = GEYSER + 1,
  NET             = CONCUSSIVE + 1,
  TORPEDO         = NET + 1,
  SHRIMP          = TORPEDO + 1,
  RED_KEY         = SHRIMP + 1,
  BLUE_KEY        = RED_KEY + 1,
  YELLOW_KEY      = BLUE_KEY + 1,
  KRAB_BOSS       = YELLOW_KEY + 1,
  SHARKMAN        = KRAB_BOSS + 1,
};
