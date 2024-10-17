#include "player_controls.hpp"

#include "collision_system.hpp"
#include "oxygen_system.hpp"
#include "physics_system.hpp"
#include "player_factories.hpp"

/**
 * @brief Checks whether or not the spawn is valid or invalid based on spawn
 * collisons The entity should already have the position attached
 *
 * @param entity - weapon to check
 * @return true if valid, false otherwise
 */
static bool checkWeaponCollisions(Entity entity) {
  if (!registry.positions.has(entity)) {
    return false;
  }
  const Position& entityPos = registry.positions.get(entity);

  // Entities can't spawn in walls
  for (Entity wall : registry.activeWalls.entities) {
    if (!registry.positions.has(wall)) {
      continue;
    }
    const Position wallPos = registry.positions.get(wall);
    if (box_collides(entityPos, wallPos)) {
      return false;
    }
  }

  return true;
}
/**
 * @brief Handles player movement
 *
 * @param key
 * @param action
 * @param mod
 * @param player
 * @return
 */
bool player_movement(int key, int action, int mod, Entity& player) {
  // Player movement attributes
  // Player oxygen attributes
  Player& keys          = registry.players.get(player);
  Oxygen& player_oxygen = registry.oxygen.get(player);

  // WASD Movement Keys
  if (!registry.deathTimers.has(player)) {
    if (key == GLFW_KEY_W) {
      if (action == GLFW_RELEASE) {
        keys.upHeld = false;
      } else {
        keys.upHeld = true;
      }
    }
    if (key == GLFW_KEY_S) {
      if (action == GLFW_RELEASE) {
        keys.downHeld = false;
      } else {
        keys.downHeld = true;
      }
    }
    if (key == GLFW_KEY_A) {
      if (action == GLFW_RELEASE) {
        keys.leftHeld = false;
      } else {
        keys.leftHeld = true;
      }
    }
    if (key == GLFW_KEY_D) {
      if (action == GLFW_RELEASE) {
        keys.rightHeld = false;
      } else {
        keys.rightHeld = true;
      }
    }
  }

  // Dashing (In case shift is held)
  if (key == GLFW_KEY_LEFT_SHIFT) {
    if (action == GLFW_PRESS) {
      registry.players.get(player).dashing = true;
      player_oxygen.rate                   = PLAYER_OXYGEN_RATE * 3;
      registry.sounds.insert(Entity(), Sound(dash_sound));
    } else if (action == GLFW_RELEASE) {
      registry.players.get(player).dashing = false;
      player_oxygen.rate                   = PLAYER_OXYGEN_RATE;
    }
  }

  return true;
}

bool player_mouse(int button, int action, int mods, Entity& player,
                  Entity& player_weapon, Entity& player_projectile) {
  // Shooting the projectile
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS &&
        registry.playerProjectiles.get(player_projectile).is_loaded) {
      if (registry.deathTimers.has(player)) {
        return false;
      }

      if (!checkWeaponCollisions(player_projectile)) {
        return false;
      }

      if (!checkWeaponCollisions(player_weapon)) {
        return false;
      }
      setFiredProjVelo(player_projectile);
      modifyOxygen(player, player_weapon);
    }
  }

  return true;
}
