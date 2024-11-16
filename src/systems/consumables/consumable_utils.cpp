#include "consumable_utils.hpp"

#include "oxygen_system.hpp"
#include "tiny_ecs_registry.hpp"
#include <player_hud.hpp>

bool handle_consumable_collisions(Entity& player, Entity& consumable, RenderSystem* renderer) {
  if (!registry.players.has(player) || !registry.consumables.has(consumable)) {
    return false;
  }

  if (registry.deathTimers.has(player)) {
    return false;
  }

  // TODO: add more affects M2+
  if (registry.weaponDrops.has(consumable)) {
    INVENTORY  type = registry.weaponDrops.get(consumable).type;
    Inventory& inv  = registry.inventory.get(player);
    if (type == INVENTORY::NET) {
      inv.nets++;
      updateInventoryCounter(renderer, INVENTORY::NET);
    } else if (type == INVENTORY::CONCUSSIVE) {
      inv.concussors++;
      updateInventoryCounter(renderer, INVENTORY::CONCUSSIVE);
    } else if (type == INVENTORY::TORPEDO) {
      inv.torpedos++;
      updateInventoryCounter(renderer, INVENTORY::TORPEDO);
    } else if (type == INVENTORY::SHRIMP) {
      inv.shrimp++;
      updateInventoryCounter(renderer, INVENTORY::SHRIMP);
    }
  }

  // will add oxygen to the player if it exists
  modifyOxygen(player, consumable);
  registry.remove_all_components_of(consumable);

  return true;
}
