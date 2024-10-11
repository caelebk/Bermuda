#include "spawning.hpp"

/**
 * @brief executes a config used to generate things in a level
 *
 * @param funcs - vector of functions to run
 */
void execute_config(const std::vector<std::function<void()>> &funcs) {
  for (const auto &func : funcs) {
    func(); // Call each function
  }
}

/**
 * @brief executes a config, which each thing in the config having a chance to
 * happen
 *
 * @param chance - float [0,1] representing the chance that something will
 * execuute
 * @param funcs - vector of functions to run
 */
void execute_config_rand(float chance,
                         const std::vector<std::function<void()>> &funcs) {
  std::default_random_engine rng;
  std::uniform_real_distribution<float> uniform_dist; // number between 0..1
  for (const auto &func : funcs) {
    if (chance >= uniform_dist(rng)) {
      func();
    }
  }
}
