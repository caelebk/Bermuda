#include "player_controls.hpp"

#include "collision_system.hpp"
#include "oxygen_system.hpp"
#include "physics_system.hpp"
#include "player_factories.hpp"
#include "player_hud.hpp"
#include "tiny_ecs_registry.hpp"

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
    // If it's a crate, let the player blast it.
    if (registry.breakables.has(wall)) {
      continue;
    }
    const Position wallPos = registry.positions.get(wall);
    if (box_collides(entityPos, wallPos)) {
      return false;
    }
  }

  return true;
}

float player_texture_num = 0.f;
/**
 * @brief Handles player movement
 *
 * @param key
 * @param action
 * @param mod
 * @param player
 * @return
 */
bool player_movement(int key, int action, int mod) {
  // Player movement attributes
  // Player oxygen attributes
  if (!registry.players.has(player) || !registry.oxygen.has(player)) {
    return false;
  }

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
    // if any key is held, increment player_texture_num
    if (keys.upHeld || keys.downHeld || keys.leftHeld || keys.rightHeld) {
      player_texture_num += 0.2f;
      if (player_texture_num >= 8.f) {
        player_texture_num = 0.f;
      }
    }
    // choose correct player texture based on player_texture_num
    if (player_texture_num < 2.f) {
      registry.renderRequests.get(player).used_texture =
          TEXTURE_ASSET_ID::PLAYER1;
    } else if (player_texture_num < 4.f) {
      registry.renderRequests.get(player).used_texture =
          TEXTURE_ASSET_ID::PLAYER2;
    } else if (player_texture_num < 6.f) {
      registry.renderRequests.get(player).used_texture =
          TEXTURE_ASSET_ID::PLAYER3;
    } else {
      registry.renderRequests.get(player).used_texture =
          TEXTURE_ASSET_ID::PLAYER2;
    }
  }

  // Dashing (In case shift is held)
  if (key == GLFW_KEY_LEFT_SHIFT) {
    if (action == GLFW_PRESS) {
      registry.players.get(player).gliding = true;
      player_oxygen.rate                   = PLAYER_OXYGEN_RATE * 3;
      if (!registry.sounds.has(player)) {
        registry.sounds.insert(Entity(), Sound(SOUND_ASSET_ID::PLAYER_GLIDE));
      }
    } else if (action == GLFW_RELEASE) {
      registry.players.get(player).gliding = false;
      player_oxygen.rate                   = PLAYER_OXYGEN_RATE;
    }
  }

  if (key == GLFW_KEY_SPACE) {
    bool atLeastOneKey =
        keys.upHeld || keys.downHeld || keys.leftHeld || keys.rightHeld;
    if (action == GLFW_PRESS && atLeastOneKey) {
      if (registry.players.get(player).dashCooldownTimer <= 0) {
        Player&         player_comp = registry.players.get(player);
        Entity          burstCost   = Entity();
        OxygenModifier& oxyBurstCost =
            registry.oxygenModifiers.emplace(burstCost);
        oxyBurstCost.amount = PLAYER_DASH_COST;
        player_comp.dashing = true;
        if (registry.colors.has(player_comp.dashIndicator)) {
          registry.colors.get(player_comp.dashIndicator) = vec3(0.1f);
        }
        modifyOxygen(player, burstCost);
      }
    }
  }

  return true;
}

bool player_mouse(RenderSystem* renderer, int button, int action, int mods,
                  Entity& default_wep, Entity& default_gun) {
  // Shooting the projectile
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (!registry.playerProjectiles.has(player_projectile)) {
      return false;
    }
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
      PROJECTILES type = registry.playerProjectiles.get(player_projectile).type;

      // Debug statement
      // printf("Weapon type: %d\n", (unsigned int) type);

      setFiredProjVelo();
      modifyOxygen(player, player_weapon);

      bool successfulInventoryUpdate = updateInventory(renderer, type);
      // return false if inventory failed.
      if (!successfulInventoryUpdate) {
        return successfulInventoryUpdate;
      }
    } else if (action == GLFW_PRESS &&
               !registry.playerProjectiles.get(player_projectile).is_loaded) {
      registry.sounds.insert(Entity(), Sound(SOUND_ASSET_ID::PLAYER_EMPTY_GUN));
      multiFireDialogue(renderer);
    }
  }

  return true;
}

bool player_scroll(double xOffset, double yOffset) {
  if (!yOffset) return false;
  int curr_wep_type = int(wep_type);
  if (yOffset > 0) {
    curr_wep_type++;
  } else if (yOffset < 0) {
    curr_wep_type--;
  }
  if (curr_wep_type == int(PROJECTILES::PROJ_COUNT)) {
    curr_wep_type = 0;
  } else if (curr_wep_type == -1) {
    curr_wep_type = int(PROJECTILES::SHRIMP);
  }

  PROJECTILES new_wep_type = PROJECTILES(curr_wep_type);

  Inventory& inv = registry.inventory.get(player);
  if (new_wep_type == PROJECTILES::NET && !inv.nets) {
    wep_type = PROJECTILES::NET;
    return player_scroll(xOffset, yOffset);
  } else if (new_wep_type == PROJECTILES::CONCUSSIVE && !inv.concussors) {
    wep_type = PROJECTILES::CONCUSSIVE;
    return player_scroll(xOffset, yOffset);
  } else if (new_wep_type == PROJECTILES::TORPEDO && !inv.torpedos) {
    wep_type = PROJECTILES::TORPEDO;
    return player_scroll(xOffset, yOffset);
  } else if (new_wep_type == PROJECTILES::SHRIMP && !inv.shrimp) {
    wep_type = PROJECTILES::SHRIMP;
    return player_scroll(xOffset, yOffset);
  }

  if (new_wep_type == PROJECTILES::HARPOON) {
    doWeaponSwap(harpoon, harpoon_gun, PROJECTILES::HARPOON);
    changeSelectedCounterColour(INVENTORY::HARPOON);
  } else if (new_wep_type == PROJECTILES::NET) {
    doWeaponSwap(net, net_gun, PROJECTILES::NET);
    changeSelectedCounterColour(INVENTORY::NET);
  } else if (new_wep_type == PROJECTILES::CONCUSSIVE) {
    doWeaponSwap(concussive, concussive_gun, PROJECTILES::CONCUSSIVE);
    changeSelectedCounterColour(INVENTORY::CONCUSSIVE);
  } else if (new_wep_type == PROJECTILES::TORPEDO) {
    doWeaponSwap(torpedo, torpedo_gun, PROJECTILES::TORPEDO);
    changeSelectedCounterColour(INVENTORY::TORPEDO);
  } else if (new_wep_type == PROJECTILES::SHRIMP) {
    doWeaponSwap(shrimp, shrimp_gun, PROJECTILES::SHRIMP);
    changeSelectedCounterColour(INVENTORY::SHRIMP);
  }

  return true;
}

/*
Update inventory based on projectile being fired
Returns true if had inventory to shoot, returns false if no inventory to shoot.
*/
bool updateInventory(RenderSystem* renderer, PROJECTILES type) {
  Inventory& inv = registry.inventory.get(player);
  switch (type) {
    case PROJECTILES::NET:
      if (!inv.nets) {
        return false;
      }
      inv.nets--;
      updateInventoryCounter(renderer, INVENTORY::NET);
      if (!inv.nets) {
        doWeaponSwap(harpoon, harpoon_gun, PROJECTILES::HARPOON);
        changeSelectedCounterColour(INVENTORY::HARPOON);
      }
      break;
    case PROJECTILES::CONCUSSIVE:
      if (!inv.concussors) {
        return false;
      }
      inv.concussors--;
      updateInventoryCounter(renderer, INVENTORY::CONCUSSIVE);
      // if inv.concussors is 0 or less, it'll be handled in debuff.cpp
      break;
    case PROJECTILES::TORPEDO:
      if (!inv.torpedos) {
        return false;
      }
      inv.torpedos--;
      updateInventoryCounter(renderer, INVENTORY::TORPEDO);
      if (!inv.torpedos) {
        doWeaponSwap(harpoon, harpoon_gun, PROJECTILES::HARPOON);
        changeSelectedCounterColour(INVENTORY::HARPOON);
      }
      break;
    case PROJECTILES::SHRIMP:
      if (!inv.shrimp) {
        return false;
      }
      inv.shrimp--;
      updateInventoryCounter(renderer, INVENTORY::SHRIMP);
      if (!inv.shrimp) {
        doWeaponSwap(harpoon, harpoon_gun, PROJECTILES::HARPOON);
        changeSelectedCounterColour(INVENTORY::HARPOON);
      }
      break;
  }
  // Debug Statements:
  // printf("Nets: %d\n", inv.nets);
  // printf("Concussive charges: %d\n", inv.concussors);
  // printf("Torpedos: %d\n", inv.torpedos);
  // printf("Shrimp Charges: %d\n\n", inv.shrimp);
  return true;
}

void swapWeps(Entity swapped, Entity swapper, PROJECTILES projectile) {
  if (registry.playerProjectiles.get(swapped).is_loaded) {
    destroyGunOrProjectile(swapped);
  }

  vec2             scale      = HARPOON_SCALE_FACTOR * HARPOON_BOUNDING_BOX;
  TEXTURE_ASSET_ID texture_id = TEXTURE_ASSET_ID::HARPOON;

  switch (projectile) {
    case PROJECTILES::NET:
      scale      = NET_SCALE_FACTOR * NET_BOUNDING_BOX;
      texture_id = TEXTURE_ASSET_ID::NET;
      break;
    case PROJECTILES::CONCUSSIVE:
      scale      = CONCUSSIVE_SCALE_FACTOR * CONCUSSIVE_BOUNDING_BOX;
      texture_id = TEXTURE_ASSET_ID::CONCUSSIVE;
      break;
    case PROJECTILES::TORPEDO:
      scale      = TORPEDO_SCALE_FACTOR * TORPEDO_BOUNDING_BOX;
      texture_id = TEXTURE_ASSET_ID::TORPEDO;
      break;
    case PROJECTILES::SHRIMP:
      scale      = SHRIMP_SCALE_FACTOR * SHRIMP_BOUNDING_BOX;
      texture_id = TEXTURE_ASSET_ID::SHRIMP;
      break;
  }

  // None of this is necessary if the swapper projectile hasn't collided yet
  if (registry.playerProjectiles.get(swapper).is_loaded) {
    // Setting initial positon values
    Position& position = registry.positions.emplace(swapper);
    position.scale     = scale;

    Position& player_pos = registry.positions.get(player);
    if (player_pos.scale.x < 0) {
      position.scale.x *= -1;
    }

    // Setting initial motion values
    // Motion will be used when acting as a projectile and is not loaded into a
    // Gun
    Motion& motion      = registry.motions.emplace(swapper);
    motion.velocity     = {0.f, 0.f};
    motion.acceleration = {0, 0};

    // Request Render
    registry.renderRequests.insert(
        swapper,
        {texture_id, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});
  }
}

void handleGunSwap(Entity swapped, Entity swapper, PROJECTILES projectile) {
  destroyGunOrProjectile(swapped);

  vec2             scale      = GUN_SCALE_FACTOR * GUN_BOUNDING_BOX;
  TEXTURE_ASSET_ID texture_id = TEXTURE_ASSET_ID::HARPOON_GUN;

  switch (projectile) {
    case PROJECTILES::NET:
      scale      = NET_GUN_SCALE_FACTOR * NET_GUN_BOUNDING_BOX;
      texture_id = TEXTURE_ASSET_ID::NET_GUN;
      break;
    case PROJECTILES::CONCUSSIVE:
      scale      = CONCUSSIVE_GUN_SCALE_FACTOR * CONCUSSIVE_GUN_BOUNDING_BOX;
      texture_id = TEXTURE_ASSET_ID::CONCUSSIVE_GUN;
      break;
    case PROJECTILES::TORPEDO:
      scale      = TORPEDO_GUN_SCALE_FACTOR * TORPEDO_GUN_BOUNDING_BOX;
      texture_id = TEXTURE_ASSET_ID::TORPEDO_GUN;
      break;
    case PROJECTILES::SHRIMP:
      scale      = SHRIMP_GUN_SCALE_FACTOR * SHRIMP_GUN_BOUNDING_BOX;
      texture_id = TEXTURE_ASSET_ID::SHRIMP_GUN;
      break;
  }

  // Setting initial position values
  Position& position = registry.positions.emplace(swapper);
  position.scale     = scale;

  Position& player_pos = registry.positions.get(player);
  if (player_pos.scale.x < 0) {
    position.scale.x *= -1;
  }

  // Setting initial motion values
  Motion& motion      = registry.motions.emplace(swapper);
  motion.velocity     = {0.f, 0.f};
  motion.acceleration = {0, 0};

  // Request Render
  registry.renderRequests.insert(swapper, {texture_id, EFFECT_ASSET_ID::PLAYER,
                                           GEOMETRY_BUFFER_ID::SPRITE});
}

void handleWeaponSwapping(RenderSystem* renderer, int key) {
  if (!registry.inventory.has(player)) {
    return;
  }
  Inventory& inv = registry.inventory.get(player);

  // Switch to harpoon gun
  if (key == GLFW_KEY_1 && player_projectile != harpoon) {
    doWeaponSwap(harpoon, harpoon_gun, PROJECTILES::HARPOON);
    changeSelectedCounterColour(INVENTORY::HARPOON);
  }

  // Switch to Net
  if (key == GLFW_KEY_2 && inv.nets && player_projectile != net) {
    doWeaponSwap(net, net_gun, PROJECTILES::NET);
    changeSelectedCounterColour(INVENTORY::NET);
  } else if (key == GLFW_KEY_2 && !inv.nets && player_projectile != net) {
    weaponSwitchingDisabledDialogue(renderer);
  }

  // Switch to Concussive
  if (key == GLFW_KEY_3 && inv.concussors && player_projectile != concussive) {
    doWeaponSwap(concussive, concussive_gun, PROJECTILES::CONCUSSIVE);
    changeSelectedCounterColour(INVENTORY::CONCUSSIVE);
  } else if (key == GLFW_KEY_3 && !inv.concussors && player_projectile != concussive) {
    weaponSwitchingDisabledDialogue(renderer);
  }

  // Switch to Torpedo
  if (key == GLFW_KEY_4 && inv.torpedos && player_projectile != torpedo) {
    doWeaponSwap(torpedo, torpedo_gun, PROJECTILES::TORPEDO);
    changeSelectedCounterColour(INVENTORY::TORPEDO);
  } else if (key == GLFW_KEY_4 && !inv.torpedos && player_projectile != torpedo) {
    weaponSwitchingDisabledDialogue(renderer);
  }

  // Switch to Pistol Shrimp
  if (key == GLFW_KEY_5 && inv.shrimp && player_projectile != shrimp) {
    doWeaponSwap(shrimp, shrimp_gun, PROJECTILES::SHRIMP);
    changeSelectedCounterColour(INVENTORY::SHRIMP);
  } else if (key == GLFW_KEY_5 && !inv.shrimp && player_projectile != shrimp) {
    weaponSwitchingDisabledDialogue(renderer);
  }
}

// Helper for handleWeaponSwapping and player_mouse
void doWeaponSwap(Entity swapper_proj, Entity swapper_wep,
                  PROJECTILES projectile) {
  swapWeps(player_projectile, swapper_proj, projectile);
  handleGunSwap(player_weapon, swapper_wep, projectile);
  player_projectile = swapper_proj;
  player_weapon     = swapper_wep;
  wep_type          = projectile;
}

bool destroyGunOrProjectile(Entity entity) {
  bool check_proj_comps = registry.motions.has(entity) &&
                          registry.positions.has(entity) &&
                          registry.renderRequests.has(entity);
  if (check_proj_comps) {
    registry.motions.remove(entity);
    registry.positions.remove(entity);
    registry.renderRequests.remove(entity);
    return true;
  }
  return false;
}
