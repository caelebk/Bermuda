#include "map_util.hpp"

#include "oxygen_system.hpp"

bool handle_interactable_collisions(Entity& player, Entity& interactable) {
  if (!registry.players.has(player) ||
      !registry.interactable.has(interactable)) {
    return false;
  }

  if (registry.deathTimers.has(player)) {
    return false;
  }

  // TODO: add more affects M2+

  // will add oxygen to the player if it exists
  modifyOxygen(player, interactable);

  return true;
}
