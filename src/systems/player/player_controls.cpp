#include "player_controls.hpp"
#include "player_factories.hpp"

/**
 * @brief Handles player movement
 *
 * @param key
 * @param action
 * @param mod
 * @param player
 * @return
 */
bool player_movement(int key, int action, int mod, Entity &player) {
  // Player movement attributes
  // Player oxygen attributes
  Player &keys = registry.players.get(player);
  Oxygen &player_oxygen = registry.oxygen.get(player);

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
      player_oxygen.rate = PLAYER_OXYGEN_RATE * 3;
    } else if (action == GLFW_RELEASE) {
      registry.players.get(player).dashing = false;
      player_oxygen.rate = PLAYER_OXYGEN_RATE;
    }
  }

  return true;
}
