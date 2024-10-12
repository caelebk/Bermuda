#include "consumable_utils.hpp"
#include "oxygen_system.hpp"
#include "tiny_ecs_registry.hpp"

bool handle_consumable_collisions(Entity &player, Entity &consumable) {
  if (!registry.players.has(player) || !registry.consumables.has(consumable)) {
    return false;
  }

  if (registry.deathTimers.has(player)) {
    return false;
  }

  // TODO: add more affects M2+

  // will add oxygen to the player if it exists
  consumeOxygen(player, consumable);
  registry.remove_all_components_of(consumable);

  return true;
}
