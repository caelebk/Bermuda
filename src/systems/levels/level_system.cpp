#include "level_system.hpp"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <string>

#include "common.hpp"
#include "graph.hpp"
#include "level_spawn.hpp"
#include "map_factories.hpp"
#include "player_factories.hpp"
#include "spawning.hpp"
#include "level_factories.hpp"

LevelSystem::LevelSystem() {};

void LevelSystem::init(RenderSystem* renderer, LevelBuilder* level) {
  this->renderer = renderer;
  this->level = level;
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
  execute_config_rand(
      level->get_room_by_editor_id(current_room_editor_id).boss_spawn_function,
      level->get_room_by_editor_id(current_room_editor_id), renderer);
}

void LevelSystem::activate_current_room() {
  RoomBuilder& current_room = level->get_room_by_editor_id(current_room_editor_id);

  // Render the walls.
  for (Entity& wall : registry.spaces.get(current_room.entity).walls) {
    // activate_boundary(wall);
    registry.activeWalls.emplace(wall);
    registry.renderRequests.insert(
        wall, {TEXTURE_ASSET_ID::WALL, EFFECT_ASSET_ID::TEXTURED,
               GEOMETRY_BUFFER_ID::SPRITE});
  }

  // Render the doors.
  for (Entity& door : registry.spaces.get(current_room.entity).doors) {
    // activate_boundary(door);
    registry.activeDoors.emplace(door);
    Direction direction = registry.doorConnections.get(door).direction;
    Position& door_position = registry.positions.get(door);
    if (direction == Direction::SOUTH || direction == Direction::WEST) {
      door_position.angle = M_PI;
    }
    if (direction == Direction::NORTH || direction == Direction::SOUTH) {
      registry.renderRequests.insert(door, {TEXTURE_ASSET_ID::DOORWAY_H,
                                            EFFECT_ASSET_ID::TEXTURED,
                                            GEOMETRY_BUFFER_ID::SPRITE});
    } else {
      registry.renderRequests.insert(door, {TEXTURE_ASSET_ID::DOORWAY_V,
                                            EFFECT_ASSET_ID::TEXTURED,
                                            GEOMETRY_BUFFER_ID::SPRITE});
    }
  }

  // Render the floor.
  Entity    floor     = Entity();
  Position& floor_pos = registry.positions.emplace(floor);
  floor_pos.position  = {window_width_px / 2, window_height_px / 2};
  floor_pos.scale     = {window_width_px, window_height_px};

  registry.floors.emplace(floor);

  registry.renderRequests.insert(
      floor, {TEXTURE_ASSET_ID::FLOOR, EFFECT_ASSET_ID::TEXTURED,
              GEOMETRY_BUFFER_ID::SPRITE});

  // Spawn the appropriate entities.
  if (current_room.has_entered) {
    // Retrieve all the saved entities and respawn them as they were
    while (current_room.saved_entities.size() > 0) {
      EntitySave es = current_room.saved_entities.back();
      es.respawn(renderer);
      current_room.saved_entities.pop_back();
    }
  } else {
    if (current_room.is_boss_room) {
      current_room.has_entered = true;
      spawn_miniboss();
    } else {
      current_room.has_entered = true;
      // It's the first time in this room, spawn for the first time.
      execute_config_rand(current_room.room_spawn_function, current_room, renderer);
      execute_config_rand_chance(current_room.room_spawn_function, current_room, renderer, 0.5);
    }
  }
}

void LevelSystem::deactivate_current_room() {
  // clear, may have entities from a restart
  level->get_room_by_editor_id(current_room_editor_id).saved_entities.clear();

  for (auto& boundary :
       registry.spaces.get(level->get_room_by_editor_id(current_room_editor_id).entity).boundaries) {
    deactivate_boundary(boundary);
  }

  while (registry.floors.entities.size() > 0) {
    registry.remove_all_components_of(registry.floors.entities.back());
  }

  while (registry.deadlys.entities.size() > 0) {
    Entity e = registry.deadlys.entities.back();
    level->get_room_by_editor_id(current_room_editor_id).saved_entities.push_back(EntitySave(e));
    registry.remove_all_components_of(e);
  }

  while (registry.consumables.entities.size() > 0) {
    Entity e = registry.consumables.entities.back();
    level->get_room_by_editor_id(current_room_editor_id).saved_entities.push_back(EntitySave(e));
    registry.remove_all_components_of(e);
  }

  while (registry.items.entities.size() > 0) {
    Entity e = registry.items.entities.back();
    level->get_room_by_editor_id(current_room_editor_id).saved_entities.push_back(EntitySave(e));
    registry.remove_all_components_of(e);
  }

  while (registry.interactable.entities.size() > 0) {
    Entity e = registry.interactable.entities.back();
    level->get_room_by_editor_id(current_room_editor_id).saved_entities.push_back(EntitySave(e));
    registry.remove_all_components_of(e);
  }

  while (registry.breakables.entities.size() > 0) {
    Entity e = registry.breakables.entities.back();
    level->get_room_by_editor_id(current_room_editor_id).saved_entities.push_back(EntitySave(e));
    registry.remove_all_components_of(e);
  }

  registry.collisions.clear();
}

void LevelSystem::move_player_to_door(Direction direction, Entity& exit_door) {
  for (auto& door : registry.spaces.get(level->get_room_by_editor_id(current_room_editor_id).entity).doors) {
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

  deactivate_current_room();
  set_current_room_editor_id(room_id);
  move_player_to_door(direction, exit_door);
  activate_current_room();

  std::cout << "Player entered room: " << current_room_editor_id << std::endl;
}

void LevelSystem::activate_starting_room() {
  // Because of the double restarts.
  level->mark_all_rooms_unvisited();

  deactivate_current_room();
  set_current_room_editor_id(STARTING_ROOM_EDITOR_ID);
  activate_current_room();
}
