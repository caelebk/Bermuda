#include "player_controls.hpp"

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
  Motion &player_motion = registry.motions.get(player);
  // WASD Movement Keys
  if (key == GLFW_KEY_W) {
    if (action == GLFW_RELEASE) {
      player_motion.velocity.y = 0;
    } else {
      player_motion.velocity.y = -SPEED_INC;
    }
  }
  if (key == GLFW_KEY_S) {
    if (action == GLFW_RELEASE) {
      player_motion.velocity.y = 0;
    } else {
      player_motion.velocity.y = SPEED_INC;
    }
  }
  if (key == GLFW_KEY_A) {
    if (action == GLFW_RELEASE) {
      player_motion.velocity.x = 0;
    } else {
      player_motion.velocity.x = -SPEED_INC;
    }
  }
  if (key == GLFW_KEY_D) {
    if (action == GLFW_RELEASE) {
      player_motion.velocity.x = 0;
    } else {
      player_motion.velocity.x = SPEED_INC;
    }
  }

  return true;
}
