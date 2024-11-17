#include "spawning.hpp"

#include <iostream>

#include "random.hpp"
#include "tiny_ecs_registry.hpp"

void execute_config_rand(
    const std::vector<std::function<Entity(RenderSystem *r, vec2 p, bool b)>>
        &funcs,
    RoomBuilder &room_builder, RenderSystem *renderer) {
  for (const auto &func : funcs) {
    vec2 loc;
    do {
      loc = room_builder.get_random_position();
    } while ((unsigned int)func(renderer, loc, true) ==
             0);  // Call each function until spawn is successful
  }
}

void execute_config_rand_chance(
    const std::vector<std::function<Entity(RenderSystem *r, vec2 p, bool b)>>
        &funcs,
    RoomBuilder &room_builder, RenderSystem *renderer, float chance) {
  for (const auto &func : funcs) {
    if (randomSuccess(chance)) {
      vec2 loc;
      do {
        loc = room_builder.get_random_position();
      } while ((unsigned int)func(renderer, loc, true) ==
               0);  // Call each function until spawn is successful
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

  while (registry.entityGroups.entities.size() > 0) {
    registry.remove_all_components_of(registry.entityGroups.entities.back());
  }

  while (registry.groups.entities.size() > 0) {
    registry.remove_all_components_of(registry.groups.entities.back());
  }

  while (registry.bubbles.entities.size() > 0) {
    registry.remove_all_components_of(registry.bubbles.entities.back());
  }

  while (registry.textRequests.entities.size() > 0) {
    registry.remove_all_components_of(registry.textRequests.entities.back());
  }

  registry.stunned.clear();
  registry.knockedback.clear();
  registry.collisions.clear();

  return true;
}

void execute_pack_spawning(
    std::function<Entity(RenderSystem* r, vec2 p, bool b)> spawnFn,
    RoomBuilder& room_builder, RenderSystem* renderer, int pack_size) {
  float width  = randomFloat(MIN_PACK_SHAPE_SIZE, MAX_PACK_SHAPE_SIZE);
  float height = randomFloat(MIN_PACK_SHAPE_SIZE, MAX_PACK_SHAPE_SIZE);
  execute_pack_spawning_area_size(spawnFn, room_builder, renderer, pack_size,
                                  width, height);
}

void execute_pack_spawning_area_size(
    std::function<Entity(RenderSystem* r, vec2 p, bool b)> spawnFn,
    RoomBuilder& room_builder, RenderSystem* renderer, int pack_size,
    float width, float height) {
  Position tempPosition;
  tempPosition.scale.x = width;
  tempPosition.scale.y = height;
  int attempts         = MAX_SPAWN_ATTEMPTS;
  do {
    tempPosition.position = room_builder.get_random_position();
    attempts--;
  } while (attempts >= 0 &&
           !checkEnemySpawnCollisions(
               tempPosition));  // Call each function until spawn is successful

  if (attempts < 0) {
    // spawning failed
    return;
  }

  // transform to be in the corner instead of the center
  tempPosition.position.x -= tempPosition.scale.x * 0.5;
  tempPosition.position.y -= tempPosition.scale.y * 0.5;

  // spawn in pack
  Entity groupEntity = Entity();
  Group& group       = registry.groups.emplace(groupEntity);

  for (int i = 0; i < pack_size; i++) {
    vec2 loc;
    for (int attempt = 0; attempt < MAX_SPAWN_ATTEMPTS; attempt++) {
      loc = {randomFloat(1.0, tempPosition.scale.x),
             randomFloat(1.0, tempPosition.scale.y)};
      loc += tempPosition.position;

      if (!room_builder.is_in_room(loc)) {
        continue;
      }

      Entity e = spawnFn(renderer, loc, true);

      if ((unsigned int)e == 0) {
        // spawning failed
        continue;
      }

      // add to group
      EntityGroup& eg = registry.entityGroups.emplace(e);
      eg.group        = groupEntity;
      // give some variance to things moving in the group
      eg.active_dir_cd = randomFloat(0.f, eg.change_dir_cd);
      group.members.push_back(e);
      break;
    }
  }
  // leave this here to debug group spawning location
  //
  // tempPosition.position.y += tempPosition.scale.y * 2.0;
  // tempPosition.position.x += tempPosition.scale.x * 2.0;
  // Entity ent = Entity();
  // registry.positions.insert(ent, tempPosition);
  // registry.floors.emplace(ent);
  // registry.renderRequests.insert(ent, {TEXTURE_ASSET_ID::JELLY,
  // EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE});
}
