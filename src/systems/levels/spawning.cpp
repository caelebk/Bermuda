#include "spawning.hpp"

#include <iostream>
#include "random.hpp"
#include "tiny_ecs_registry.hpp"

void execute_config_rand(
    const std::initializer_list<std::function<Entity(RenderSystem *r, vec2 p, bool b)>>
        &funcs,
    RoomBuilder &room_builder, RenderSystem *renderer) {
  for (const auto &func : funcs) {
    vec2 loc;
    do {
      loc = room_builder.get_random_position();
    } while ((unsigned int)func(renderer, loc, true) ==
             0); // Call each function until spawn is successful
  }
}

void execute_config_rand_chance(
    const std::initializer_list<std::function<Entity(RenderSystem *r, vec2 p, bool b)>>
        &funcs,
    RoomBuilder &room_builder, RenderSystem *renderer, float chance) {
  for (const auto &func : funcs) {
    if (randomSuccess(chance)) {
      vec2 loc;
      do {
        loc = room_builder.get_random_position();
      } while ((unsigned int)func(renderer, loc, true) == 0); // Call each function until spawn is successful
    }
  }
}

/**
 * @brief executes a config used to generate things in a level
 *
 * @param funcs - vector of functions to run
 */
void execute_config_fixed(const std::vector<std::function<void()>>& funcs) {
  for (const auto& func : funcs) {
    func();  // Call each function
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
void execute_config_fixed_rand(
    float chance, const std::vector<std::function<void()>>& funcs) {
  for (const auto& func : funcs) {
    if (randomSuccess(chance)) {
      func();
    }
  }
}

bool remove_all_entities() {
  while (registry.motions.entities.size() > 0) {
    registry.remove_all_components_of(registry.motions.entities.back());
  }

  while (registry.deadlys.entities.size() > 0) {
    registry.remove_all_components_of(registry.deadlys.entities.back());
  }

  while (registry.consumables.entities.size() > 0) {
    registry.remove_all_components_of(registry.consumables.entities.back());
  }

  while (registry.interactable.entities.size() > 0) {
    registry.remove_all_components_of(registry.interactable.entities.back());
  }

  while (registry.playerWeapons.entities.size() > 0) {
    registry.remove_all_components_of(registry.playerWeapons.entities.back());
  }

  while (registry.breakables.entities.size() > 0) {
    registry.remove_all_components_of(registry.breakables.entities.back());
  }

  while (registry.playerProjectiles.entities.size() > 0) {
    registry.remove_all_components_of(
        registry.playerProjectiles.entities.back());
  }

  while (registry.enemyProjectiles.entities.size() > 0) {
    registry.remove_all_components_of(
        registry.enemyProjectiles.entities.back());
  }

  while (registry.oxygenModifiers.entities.size() > 0) {
    registry.remove_all_components_of(registry.oxygenModifiers.entities.back());
  }

  while (registry.playersCollisionMeshes.entities.size() > 0) {
    registry.remove_all_components_of(
        registry.playersCollisionMeshes.entities.back());
  }

  while (registry.playerHUD.entities.size() > 0) {
    registry.remove_all_components_of(registry.playerHUD.entities.back());
  }

  while (registry.inventoryCounters.entities.size() > 0) {
    registry.remove_all_components_of(
        registry.inventoryCounters.entities.back());
  }

  while (registry.textRequests.entities.size() > 0) {
    registry.remove_all_components_of(registry.textRequests.entities.back());
  }

  registry.stunned.clear();
  registry.knockedback.clear();
  registry.collisions.clear();

  return true;
}
