#include "level_system.hpp"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <numeric>
#include <player_hud.hpp>
#include <string>

#include "common.hpp"
#include "enemy_factories.hpp"
#include "level_factories.hpp"
#include "level_spawn.hpp"
#include "map_factories.hpp"
#include "player_factories.hpp"
#include "spawning.hpp"

LevelSystem::LevelSystem() {};

void LevelSystem::init(RenderSystem* renderer, LevelBuilder* level) {
  this->renderer = renderer;
  this->level    = level;
};

void LevelSystem::deactivate_boundary(Entity& entity) {
  if (registry.renderRequests.has(entity)) {
    registry.renderRequests.remove(entity);
  }
  if (registry.activeWalls.has(entity)) {
    registry.activeWalls.remove(entity);
  }
  if (registry.activeDoors.has(entity)) {
    registry.activeDoors.remove(entity);
  }
};

void LevelSystem::set_current_room_editor_id(std::string room_editor_id) {
  current_room_editor_id = room_editor_id;
};

void LevelSystem::spawn_miniboss() {
  for (const auto& spawn_function_group :
       level->get_room_by_editor_id(current_room_editor_id)
           .boss_spawn_function_groups) {
    execute_config_rand(spawn_function_group,
                        level->get_room_by_editor_id(current_room_editor_id),
                        renderer);
  }
}

void LevelSystem::spawn() {
  RoomBuilder& current_room =
      level->get_room_by_editor_id(current_room_editor_id);
  // std::cout << "Has entered: " << current_room.has_entered << std::endl;

  if (current_room.has_entered) {
    // Retrieve all the saved entities and respawn them as they were
    while (current_room.saved_entities.size() > 0) {
      EntitySave es = current_room.saved_entities.back();
      es.respawn(renderer);
      current_room.saved_entities.pop_back();
    }
  } else {
    current_room.has_entered = true;
    if (current_room.is_boss_room) {
      spawn_miniboss();
      for (const auto& fixed_spawn_function_group :
           current_room.room_fixed_spawn_function_groups) {
        std::cout << "Should have spawned key" << std::endl;
        execute_config_rand(fixed_spawn_function_group, current_room, renderer);
      }
    } else {
      // It's the first time in this room, spawn for the first time.
      for (const auto& spawn_function_group :
           current_room.room_pack_spawn_function_groups) {
        for (const auto& spawn_function : spawn_function_group) {
          execute_pack_spawning(spawn_function, current_room, renderer, 8);
        }
      }
      for (const auto& spawn_function_group :
           current_room.room_spawn_function_groups) {
        // TODO: remove this out of here and pull it into the correct level
        // placements.
        execute_config_rand(spawn_function_group, current_room, renderer);
        execute_config_rand_chance(spawn_function_group, current_room, renderer,
                                   0.5);
      }
      for (const auto& fixed_spawn_function_group :
           current_room.room_fixed_spawn_function_groups) {
        std::cout << "Should have spawned key" << std::endl;
        execute_config_rand(fixed_spawn_function_group, current_room, renderer);
      }
    }
  }
}

void LevelSystem::activate_walls() {
  RoomBuilder& current_room =
      level->get_room_by_editor_id(current_room_editor_id);

  for (Entity& wall : registry.spaces.get(current_room.entity).walls) {
    registry.activeWalls.emplace(wall);
    registry.renderRequests.insert(
        wall, {TEXTURE_ASSET_ID::WALL, EFFECT_ASSET_ID::TEXTURED,
               GEOMETRY_BUFFER_ID::SPRITE});
  }
}

void LevelSystem::lock_doors(Entity& door, DoorConnection& door_connection) {
  RoomBuilder& current_room =
      level->get_room_by_editor_id(current_room_editor_id);

  // Consider all doors as walls, unless we have the key for them, and
  // recalculate all of these whenever we pick up any key, or kill a boss.
  if (current_room.is_boss_room &&
      !current_room.has_entered) {  // Only lock these doors if we've not killed
                                    // the boss.
    door_connection.locked = true;
    registry.activeWalls.emplace(door);
  } else {
    if (!isKeyCollected(door_connection.key)) {
      door_connection.locked = true;
      registry.activeWalls.emplace(door);
    } else {
      // std::cout << "unlocking" << std::endl;
      door_connection.locked = false;
    }
  }
}

void LevelSystem::activate_doors() {
  RoomBuilder& current_room =
      level->get_room_by_editor_id(current_room_editor_id);

  // Activate the doors.
  for (Entity& door : registry.spaces.get(current_room.entity).doors) {
    registry.activeDoors.emplace(door);
    DoorConnection& door_connection = registry.doorConnections.get(door);
    Direction       direction       = door_connection.direction;
    Position&       door_position   = registry.positions.get(door);

    lock_doors(door, door_connection);

    TEXTURE_ASSET_ID texture;
    if (direction == Direction::SOUTH || direction == Direction::WEST) {
      door_position.angle = M_PI;
    }
    if (direction == Direction::NORTH || direction == Direction::SOUTH) {
      switch (door_connection.key) {
        case (INVENTORY::RED_KEY): {
          texture = TEXTURE_ASSET_ID::LOCKED_RED_H;
          break;
        }
        case (INVENTORY::BLUE_KEY): {
          texture = TEXTURE_ASSET_ID::LOCKED_BLUE_H;
          break;
        }
        case (INVENTORY::YELLOW_KEY): {
          texture = TEXTURE_ASSET_ID::LOCKED_YELLOW_H;
          break;
        }
        default:
          texture = TEXTURE_ASSET_ID::DOORWAY_H;
      }
      if (current_room.is_boss_room && !current_room.has_entered) {
        texture = TEXTURE_ASSET_ID::LOCKED_DOOR_H;
      }
      registry.renderRequests.insert(door, {texture, EFFECT_ASSET_ID::TEXTURED,
                                            GEOMETRY_BUFFER_ID::SPRITE});
    } else {
      switch (door_connection.key) {
        case (INVENTORY::RED_KEY): {
          texture = TEXTURE_ASSET_ID::LOCKED_RED_V;
          break;
        }
        case (INVENTORY::BLUE_KEY): {
          texture = TEXTURE_ASSET_ID::LOCKED_BLUE_V;
          break;
        }
        case (INVENTORY::YELLOW_KEY): {
          texture = TEXTURE_ASSET_ID::LOCKED_YELLOW_V;
          break;
        }
        default:
          texture = TEXTURE_ASSET_ID::DOORWAY_V;
      }
      if (current_room.is_boss_room && !current_room.has_entered) {
        texture = TEXTURE_ASSET_ID::LOCKED_DOOR_V;
      }
      registry.renderRequests.insert(door, {texture, EFFECT_ASSET_ID::TEXTURED,
                                            GEOMETRY_BUFFER_ID::SPRITE});
    }
  }
}

void LevelSystem::activate_floor() {
  RoomBuilder& current_room =
      level->get_room_by_editor_id(current_room_editor_id);

  Entity    floor     = Entity();
  Position& floor_pos = registry.positions.emplace(floor);
  floor_pos.position  = {window_width_px / 2, window_height_px / 2};
  floor_pos.scale     = {window_width_px, window_height_px};

  registry.floors.emplace(floor);

  // TODO: Temporary Floor Switching Method
  if (current_room.is_tutorial_room) {
    registry.renderRequests.insert(
        floor, {TEXTURE_ASSET_ID::TUTORIAL_FLOOR, EFFECT_ASSET_ID::TEXTURED,
                GEOMETRY_BUFFER_ID::SPRITE});
  } else {
    registry.renderRequests.insert(
        floor, {TEXTURE_ASSET_ID::FLOOR, EFFECT_ASSET_ID::TEXTURED,
                GEOMETRY_BUFFER_ID::SPRITE});
  }
}

void LevelSystem::activate_current_room() {
  activate_walls();
  activate_doors();
  activate_floor();

  spawn();
}

void LevelSystem::deactivate_current_room(bool save) {
  // clear, may have entities from a restart
  level->get_room_by_editor_id(current_room_editor_id).saved_entities.clear();

  for (auto& boundary :
       registry.spaces
           .get(level->get_room_by_editor_id(current_room_editor_id).entity)
           .boundaries) {
    deactivate_boundary(boundary);
  }

  while (registry.deadlys.entities.size() > 0) {
    Entity e = registry.deadlys.entities.back();
    if (save) {
      level->get_room_by_editor_id(current_room_editor_id)
          .saved_entities.push_back(EntitySave(e));
    }
    registry.remove_all_components_of(e);
  }

  while (registry.consumables.entities.size() > 0) {
    Entity e = registry.consumables.entities.back();
    if (save) {
      level->get_room_by_editor_id(current_room_editor_id)
          .saved_entities.push_back(EntitySave(e));
    }
    registry.remove_all_components_of(e);
  }

  while (registry.items.entities.size() > 0) {
    Entity e = registry.items.entities.back();
    if (save) {
      level->get_room_by_editor_id(current_room_editor_id)
          .saved_entities.push_back(EntitySave(e));
    }
    registry.remove_all_components_of(e);
  }

  while (registry.interactable.entities.size() > 0) {
    Entity e = registry.interactable.entities.back();
    if (save) {
      level->get_room_by_editor_id(current_room_editor_id)
          .saved_entities.push_back(EntitySave(e));
    }
    registry.remove_all_components_of(e);
  }

  while (registry.breakables.entities.size() > 0) {
    Entity e = registry.breakables.entities.back();
    if (save) {
      level->get_room_by_editor_id(current_room_editor_id)
          .saved_entities.push_back(EntitySave(e));
    }
    registry.remove_all_components_of(e);
  }

  while (registry.floors.entities.size() > 0) {
    registry.remove_all_components_of(registry.floors.entities.back());
  }

  while (registry.bubbles.entities.size() > 0) {
    registry.remove_all_components_of(registry.bubbles.entities.back());
  }

  while (registry.drops.entities.size() > 0) {
    registry.remove_all_components_of(registry.drops.entities.back());
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

  while (registry.enemyProjectiles.entities.size() > 0) {
    Entity e = registry.enemyProjectiles.entities.back();
    registry.remove_all_components_of(e);
  }

  registry.stunned.clear();
  registry.knockedback.clear();
  registry.collisions.clear();

  printf("Saved %d entities\n",
         level->get_room_by_editor_id(current_room_editor_id)
             .saved_entities.size());
}

void LevelSystem::move_player_to_door(Direction direction, Entity& exit_door) {
  for (auto& door :
       registry.spaces
           .get(level->get_room_by_editor_id(current_room_editor_id).entity)
           .doors) {
    if (door == exit_door) {
      if (registry.positions.has(door)) {
        Position& door_position = registry.positions.get(door);
        for (auto& player : registry.players.entities) {
          Position& player_position = registry.positions.get(player);

          // Offset the player from the door so they don't immediately reswitch
          // rooms. Get the opposite direction of the wall that this door was
          // on; i.e if you enter a door on the WEST, you should spawn on the
          // EAST.
          player_position.position     = door_position.position;
          Direction opposite_direction = get_opposite_direction(direction);

          float offset = PLAYER_BOUNDING_BOX.x * PLAYER_SCALE_FACTOR.x;
          switch (opposite_direction) {
            case Direction::NORTH:
              player_position.position.y += offset;
              break;
            case Direction::EAST:
              player_position.position.x -= offset;
              break;
            case Direction::SOUTH:
              player_position.position.y -= offset;
              break;
            case Direction::WEST:
              player_position.position.x += offset;
              break;
          }
          Player&   player_comp = registry.players.get(player);
          Position& player_mesh_pos =
              registry.positions.get(player_comp.collisionMesh);
          player_mesh_pos.position = player_position.position;
        }
      }
    }
  }
}

void LevelSystem::enter_room(DoorConnection& door_connection) {
  Direction   direction = door_connection.direction;
  std::string room_id   = door_connection.room_id;
  Entity&     exit_door = door_connection.exit_door;

  RoomBuilder& room = level->get_room_by_editor_id(room_id);
  std::cout << room.has_entered << std::endl;

  deactivate_current_room(true);
  set_current_room_editor_id(room_id);
  move_player_to_door(direction, exit_door);
  activate_current_room();

  std::cout << "Player entered room: " << current_room_editor_id << std::endl;
}

void LevelSystem::activate_starting_room() {
  level->mark_all_rooms_unvisited();

  deactivate_current_room(false);
  set_current_room_editor_id(STARTING_ROOM);
  activate_current_room();
}

void LevelSystem::collect_key(INVENTORY color) {
  switch (color) {
    case (INVENTORY::RED_KEY): {
      collectRedKey(renderer);
      break;
    }
    case (INVENTORY::BLUE_KEY): {
      collectBlueKey(renderer);
      break;
    }
    case (INVENTORY::YELLOW_KEY): {
      collectYellowKey(renderer);
      break;
    }
  }

  for (const auto& entity : registry.doorConnections.entities) {
    DoorConnection& door_connection = registry.doorConnections.get(entity);
    if (door_connection.locked && door_connection.key == color) {
      // Unlock this door, and stop treating it as a wall.
      door_connection.locked = false;
      if (registry.activeWalls.has(entity)) {
        registry.activeWalls.remove(entity);
      }
    }
  }
};

void LevelSystem::clear_all_state() {
  deactivate_current_room(false);
  level->mark_all_rooms_unvisited();
}

void LevelSystem::activate_from_save(std::string id) {
  // player should already be in the position we want them in so we can ignore
  // should also already be marked as visited
  // RoomBuilder &room = level->get_room_by_editor_id(id);
  // room.has_entered = true;
  set_current_room_editor_id(id);
  activate_current_room();
}
