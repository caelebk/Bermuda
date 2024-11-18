#include "consumable_utils.hpp"

#include <player_hud.hpp>

#include "oxygen_system.hpp"
#include "tiny_ecs_registry.hpp"

bool handle_consumable_collisions(Entity& player, Entity& consumable,
                                  RenderSystem* renderer) {
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
      collectNetDialogue(renderer);
    } else if (type == INVENTORY::CONCUSSIVE) {
      inv.concussors++;
      updateInventoryCounter(renderer, INVENTORY::CONCUSSIVE);
      collectConcussiveDialogue(renderer);
    } else if (type == INVENTORY::TORPEDO) {
      inv.torpedos++;
      updateInventoryCounter(renderer, INVENTORY::TORPEDO);
      collectTorpedoDialogue(renderer);
    } else if (type == INVENTORY::SHRIMP) {
      inv.shrimp++;
      updateInventoryCounter(renderer, INVENTORY::SHRIMP);
      collectShrimpDialogue(renderer);
    } else if ((type == INVENTORY::NET && inv.nets >= 999) ||
               (type == INVENTORY::CONCUSSIVE && inv.concussors >= 999) ||
               (type == INVENTORY::TORPEDO && inv.torpedos >= 999) ||
               (type == INVENTORY::SHRIMP && inv.shrimp >= 999)) {
      overCollectDialogue(renderer);
    }
  }

  // will add oxygen to the player if it exists
  modifyOxygen(player, consumable);
  registry.remove_all_components_of(consumable);

  return true;
}
