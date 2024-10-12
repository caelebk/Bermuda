#include "spawning.hpp"
#include "tiny_ecs_registry.hpp"

/**
 * @brief executes a config used to generate things in a level
 *
 * @param funcs - vector of functions to run
 */
void execute_config_fixed(const std::vector<std::function<void()>> &funcs) {
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
void execute_config_fixed_rand(float chance,
                         const std::vector<std::function<void()>> &funcs) {
  std::default_random_engine rng;
  std::uniform_real_distribution<float> uniform_dist; // number between 0..1
  for (const auto &func : funcs) {
    if (chance >= uniform_dist(rng)) {
      func();
    }
  }
}

bool remove_all_entities() {
  for (const Entity &e: registry.deadlys.entities) {
    registry.remove_all_components_of(e);
  }
  registry.deadlys.clear();

  for (const Entity &e: registry.consumables.entities) {
    registry.remove_all_components_of(e);
  }
  registry.consumables.clear();
  return true;
}
