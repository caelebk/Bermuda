#include "level_builder.hpp"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <numeric>
#include <string>

#include "level_factories.hpp"
#include "level_util.hpp"
#include "map_factories.hpp"
#include "random.hpp"
#include "room_builder.hpp"
#include "spawning.hpp"

RoomBuilder& LevelBuilder::get_room_by_editor_id(EditorID s_id) {
  return rooms[s_id];
};

void LevelBuilder::randomize_door_objective(EditorID& r1_id, EditorID& r2_id, DoorConnection& door_connection_1, DoorConnection& door_connection_2) {
  RoomBuilder& room_1 = rooms[r1_id];
  RoomBuilder& room_2 = rooms[r2_id];

  std::mt19937 rng = getGlobalRNG();

  std::shuffle(room_1.objectives.begin(), room_1.objectives.end(), rng);
  if (door_connection_1.assigned == false && door_connection_2.assigned == false) {
    for (size_t i = 0; i < room_1.objectives.size(); i++) {
      Objective& random_objective = room_1.objectives[i];

      // Pressure plate logic. Must be available in both rooms.
      if (
        random_objective == Objective::PRESSURE_PLATE
        && std::find(room_2.objectives.begin(), room_2.objectives.end(), Objective::PRESSURE_PLATE) != room_2.objectives.end()
        ) {
        room_1.spawn_wrappers.push_back(PRESSURE_PLATE_SPAWN);
        room_2.spawn_wrappers.push_back(PRESSURE_PLATE_SPAWN);
        room_1.spawn_wrappers.push_back(ROCK_SPAWN);
        room_2.spawn_wrappers.push_back(ROCK_SPAWN);
        std::cout << "assigned pressure plates to rooms: " << r1_id << " and " << r2_id << std::endl;

        // Restrict ourselves to one pressure plate per room.
        room_1.objectives.erase(std::remove(room_1.objectives.begin(), room_1.objectives.end(), Objective::PRESSURE_PLATE), room_1.objectives.end());
        room_2.objectives.erase(std::remove(room_2.objectives.begin(), room_2.objectives.end(), Objective::PRESSURE_PLATE), room_2.objectives.end());

        door_connection_1.objective = Objective::PRESSURE_PLATE;
        door_connection_2.objective = Objective::PRESSURE_PLATE;
        break;
      } else {
        continue;
      }      

      // Assign if we're good.
      door_connection_1.objective = random_objective;
      door_connection_2.objective = random_objective;
    }
    
    door_connection_1.assigned = true;
    door_connection_2.assigned = true;
  }
}

void LevelBuilder::connect(Direction direction, EditorID r_id, EditorID d1_id,
                           EditorID r2_id, EditorID d2_id) {
  RoomBuilder& room_1 = rooms[r_id];
  RoomBuilder& room_2 = rooms[r2_id];
  Entity&      door_1 = room_1.doors[d1_id];
  Entity&      door_2 = room_2.doors[d2_id];

  if (!registry.doorConnections.has(door_1) &&
      !registry.doorConnections.has(door_2)) {
    DoorConnection& door_connection_1 =
        registry.doorConnections.emplace(door_1);
    door_connection_1.direction = direction;
    door_connection_1.room_id   = r2_id;
    door_connection_1.exit_door = door_2;

    DoorConnection& door_connection_2 =
        registry.doorConnections.emplace(door_2);
    door_connection_2.direction = get_opposite_direction(direction);
    door_connection_2.room_id   = r_id;
    door_connection_2.exit_door = door_1;

    std::mt19937 rng = getGlobalRNG();

    if (!rooms[r_id].is_boss_room && rooms[r2_id].is_boss_room) { // only add the objective on the connecting door
      door_connection_1.objective = Objective::BOSS;
      door_connection_2.objective = Objective::BOSS;
    } else {
      randomize_door_objective(r_id, r2_id, door_connection_1, door_connection_2);
    }
  } else {
    if (registry.doorConnections.has(door_1) && registry.doorConnections.has(door_2)) {
      DoorConnection& door_connection1 = registry.doorConnections.get(door_1);
      DoorConnection& door_connection2 = registry.doorConnections.get(door_2);
      door_connection1.objective = door_connection2.objective;
      door_connection2.objective = door_connection1.objective;
    }
  }
};

void LevelBuilder::mark_difficulty_regions() {
  assert(ROOM_CLUSTERS.size() == ROOM_SPAWN_WRAPPERS.size());
  int room_index = 0;

  for (unsigned int i = 0; i < ROOM_CLUSTERS.size(); i++) {
    int rooms_in_cluster = ROOM_CLUSTERS[i];

    // Assign the current spawn function to all rooms in the current cluster, and also
    // assign the room name.
    for (int j = 0; j < rooms_in_cluster; j++) {
      std::string room_name;
      EditorID room_id = std::to_string(room_index);
      RoomBuilder& room = rooms[room_id];

      room.name = room_name.append(std::to_string(i)).append("-").append(std::to_string(room_index));
      room.spawn_wrappers = ROOM_SPAWN_WRAPPERS[i];
      room_index++;
    }
  }
}

void LevelBuilder::mark_tutorial_room() {
  rooms[TUTORIAL_ROOM].is_tutorial_room = true;
}

void LevelBuilder::mark_boss_rooms() {
  assert(MINIBOSS_ROOMS.size() == MINIBOSS_SPAWN_WRAPPERS.size());
  for (int i = 0; i < MINIBOSS_ROOMS.size(); i++) {
    EditorID room            = std::to_string(MINIBOSS_ROOMS[i]);
    rooms[room].is_boss_room = true;
    rooms[room].spawn_wrappers = MINIBOSS_SPAWN_WRAPPERS[i];
  }
  EditorID room = "15";
  rooms[room].is_boss_room = true;
  rooms[room].spawn_wrappers = FINAL_BOSS;
}

std::vector<int> LevelBuilder::get_random_door_positions(int num_doors, int min,
                                                         int max) {
  std::vector<int> positions;
  bool             is_valid;

  while ((int)positions.size() < num_doors) {
    int position = getRandInt(min, max);
    is_valid     = true;

    // Ensure the position is at least DOOR_SIZE units away from both min and
    // max.
    if (position <= min + DOOR_SIZE || position >= max - DOOR_SIZE) {
      is_valid = false;
    }

    // Ensure each new position is at least DOOR_SIZE units away from existing
    // ones.
    for (int existing_pos : positions) {
      if (std::abs(existing_pos - position) < DOOR_SIZE + 1) {
        is_valid = false;
        break;
      }
    }

    if (is_valid) {
      positions.push_back(position);
    } else {
      // Try again from scratch.
      positions.clear();
      continue;
    }
  }

  return positions;
}

void LevelBuilder::build_wall_with_random_doors(
    Direction direction, EditorID room_id, int max_units, int unit_size,
    std::function<void(int)>              draw_segment,
    std::function<void(std::string, int)> draw_door) {
  int                   door_index   = 0;
  int                   current_size = 0;
  std::vector<EditorID> connected_rooms =
      rooms[room_id].get_connections_with_direction(direction);
  std::vector<int> door_positions =
      get_random_door_positions(connected_rooms.size(), 0, max_units);
  while (door_index < (int)door_positions.size() && current_size < max_units) {
    int      current_door  = door_positions[door_index];
    EditorID other_room_id = connected_rooms[door_index];

    // Calculate the segment before the next door position
    int segment = current_door - current_size;

    draw_segment(unit_size * segment);
    // Generate a door with the id: <direction_enum>_<room_number>; i.e NORTH to
    // room 1 is 0_1.
    std::string door_id;
    door_id.append(other_room_id).append("_").append(std::to_string(direction));
    draw_door(door_id, DOOR_SIZE * unit_size);

    current_size += segment + DOOR_SIZE;  // Update for door and segment size
    door_index++;
  }

  // Fill the remaining wall segment if needed
  if (current_size < max_units) {
    int remaining_segment = max_units - current_size;
    draw_segment(unit_size * remaining_segment);
  }
};

void LevelBuilder::randomize_keys() {
  assert(KEYS.size() == KEY_SPAWN_WRAPPERS.size());
  std::mt19937 rng = getGlobalRNG();

  // Randomly sample rooms to place our keys in. Skip the first and last rooms,
  // and copy the traversal vector.
  std::vector<int> traversal_indices;
  for (int i = 0; i < (int)traversal.size(); i++) {
    traversal_indices.push_back(i);
  }
  std::shuffle(traversal_indices.begin(), traversal_indices.end() - 1, rng);

  // Select the necessary randomized indices.
  std::vector<int> indices;
  for (int i = 0; i < (int)KEYS.size(); i++) {
    indices.push_back(traversal_indices[i]);
  }

  // Sort them to maintain increasing traversal order.
  std::sort(indices.begin(), indices.end());

  // Select the first available indices from our shuffled copy as defined by our
  // key count.
  std::vector<EditorID> key_rooms;
  for (int i = 0; i < KEYS.size(); i++) {
    EditorID key_room = traversal[indices[i]];
    if (key_rooms.size() > KEYS.size()) {
      break;
    }
    if (key_room == STARTING_ROOM || key_room == FINAL_BOSS_ROOM) {
      continue;
    }
    key_rooms.push_back(key_room);
    std::cout << "key room: " << key_room << std::endl;
    rooms[key_room].spawn_wrappers.push_back(KEY_SPAWN_WRAPPERS[i]);
  }

  for (int i = 0; i < (int)indices.size(); i++) {
    std::cout << "index: " << indices[i] << std::endl;
  }

  // Iterate over our traversal path and mark room past the ones we defined
  // above as possibly having a door of that type. i.e key_spawns = [3,7,9] then
  // add key 1, 2, and 3 spawns to the spawn functions. For rooms past 3,
  // possibly flag them as key 1 rooms. For rooms past 7, possibly flag them as
  // key 1 or 2 rooms. For rooms past 9, possibly flag them as key 1, 2 or 3
  // rooms.
  for (int i = 0; i < (int)traversal.size(); i++) {
    EditorID room_id     = traversal[i];
    int      room_number = std::stoi(room_id);

    for (int j = 0; j < (int)indices.size(); j++) {
      EditorID key_room_id     = key_rooms[j];
      int      key_room_number = std::stoi(key_room_id);

      if (i >= indices[j]) {
        rooms[room_id].objectives.push_back(KEYS[j]);
      }
    }
  }
}

void LevelBuilder::randomize_pressure_plates() {
  int i = 0;
  for (const EditorID& id : traversal) {
    if (!rooms[id].is_boss_room && i != 0) {
      // Weight the probability vector of objectives so we actually get a few pressure plates every now and then, otherwise the keys
      // dominate and you can get runs with no pressure plates.
      rooms[id].objectives.push_back(Objective::PRESSURE_PLATE);
      rooms[id].objectives.push_back(Objective::PRESSURE_PLATE);
      rooms[id].objectives.push_back(Objective::PRESSURE_PLATE);
    }
    i++;
  }
}

void LevelBuilder::randomize_objectives() {
  randomize_keys();
  randomize_pressure_plates();

  // Debug
  // for (const auto& room_id : traversal) {
  //   const RoomBuilder& room = rooms[room_id];
  //   std::cout << "room: " << room_id << std::endl;
  //   for (const auto& objective : room.objectives) {
  //     std::cout << (int)objective << std::endl;
  //   }
  // }
}

void LevelBuilder::randomize_room_shapes() {
  for (const auto& pair : rooms) {
    EditorID                                room_id = pair.first;
    std::unordered_map<EditorID, Direction> connections =
        pair.second.connections;

    RoomBuilder& current_room = get_room_by_editor_id(room_id);

    if (current_room.is_tutorial_room) {
      // Fix the door placement. We need to change this as well as the door we already generated for the other room.
      EditorID& first_room = traversal[0]; // this is guaranteed

      // Also sneakily edit the other room.
      RoomBuilder& other_room = get_room_by_editor_id(first_room);
      other_room.connections[room_id] = Direction::SOUTH;

      // Generate the door ID we'll use.
      std::string door_id;
      door_id.append(first_room).append("_").append(std::to_string(Direction::NORTH));
      current_room.up(Y_10U).right(X_20U).down(Y_10U).left(X_6U).door(door_id, DOOR_SIZE * X_1U).left(X_13U);
    } else {
      // west
      build_wall_with_random_doors(
          Direction::WEST, room_id, MAX_Y_UNITS, Y_1U,
          [&](int segment) { current_room.up(segment); },
          [&](EditorID door_id, int door_size) {
            current_room.door(door_id, door_size);
          });

      // north
      build_wall_with_random_doors(
          Direction::SOUTH, room_id, MAX_X_UNITS, X_1U,
          [&](int segment) { current_room.right(segment); },
          [&](EditorID door_id, int door_size) {
            current_room.door(door_id, door_size);
          });

      // east
      build_wall_with_random_doors(
          Direction::EAST, room_id, MAX_Y_UNITS, Y_1U,
          [&](int segment) { current_room.down(segment); },
          [&](EditorID door_id, int door_size) {
            current_room.door(door_id, door_size);
          });

      // south
      build_wall_with_random_doors(
          Direction::NORTH, room_id, MAX_X_UNITS, X_1U,
          [&](int segment) { current_room.left(segment); },
          [&](EditorID door_id, int door_size) {
            current_room.door(door_id, door_size);
          });
    }
  }
}

void LevelBuilder::connect_doors() {
  // Make sure to do this in traversal order, or else you might get locked out
  // of rooms.
  for (const EditorID& room_1_id : traversal) {
    std::unordered_map<EditorID, Direction> connections =
        rooms[room_1_id].connections;

    for (const auto& connections_pair : connections) {
      EditorID  room_2_id = connections_pair.first;
      Direction direction = connections_pair.second;
      EditorID  door_1_id;
      EditorID  door_2_id;

      door_1_id.append(room_2_id).append("_").append(std::to_string(direction));
      door_2_id.append(room_1_id).append("_").append(
          std::to_string(get_opposite_direction(direction)));

      connect(direction, room_1_id, door_1_id, room_2_id, door_2_id);
    }
  }
}

// Note since we transition when moving spaces, these graphs don't have to be
// planar.
void LevelBuilder::generate_random_level() {
  printf("start\n");
  randomize_connections();
  printf("after random connect\n");
  randomize_connection_directions();
  printf("after random directions\n");

  // Also, mark important rooms.
  mark_difficulty_regions();
  mark_tutorial_room();
  mark_boss_rooms();

  randomize_objectives();
  printf("after random objectives\n");
  randomize_room_shapes();
  printf("after random room shapes\n");

  connect_doors();
  printf("after connect doors\n");
}

void LevelBuilder::mark_all_rooms_unvisited() {
  for (const auto& pair : rooms) {
    RoomBuilder& room = get_room_by_editor_id(pair.first);
    room.has_entered  = false;
    room.saved_entities.clear();
  }
}

int LevelBuilder::count_edges_with_direction(EditorID  room,
                                             Direction direction) {
  int count = 0;
  for (const auto& pair : rooms[room].connections) {
    if (pair.second == direction) {
      count++;
    }
  }
  return count;
}

void LevelBuilder::randomize_connection_directions() {
  std::mt19937           rng        = getGlobalRNG();
  std::vector<Direction> directions = {Direction::NORTH, Direction::EAST,
                                       Direction::SOUTH, Direction::WEST};

  // For each edge in the adjacency list, associate each to one of four
  // randomized directions. Also, expand the adjacency list into a directed
  // adjacency list that stores this assigned direction as a nested map.
  std::set<std::pair<EditorID, EditorID>> processed_pairs;

  for (const auto& pair : rooms) {
    EditorID              room_editor_id = pair.first;
    std::vector<EditorID> connected_rooms;

    // Fill connected_rooms with the relevant EditorIDs for this room.
    for (const auto& connection : pair.second.connections) {
      connected_rooms.push_back(connection.first);
    }

    for (int i = 0; i < (int)connected_rooms.size(); i++) {
      EditorID other_room_editor_id = connected_rooms[i];

      // If we've already processed this pair, move on.
      if (processed_pairs.count({other_room_editor_id, room_editor_id}) > 0) {
        continue;
      }

      Direction random_direction;
      do {
        std::shuffle(directions.begin() + 1, directions.end() - 1, rng);
        random_direction = directions[getRandInt(0, directions.size() - 1)];
      } while (
          // Keep selecting another direction until that direction has not
          // already been added to this room, up to a predefined number, just so
          // we don't get every wall on the same door.
          count_edges_with_direction(room_editor_id, random_direction) >=
          MAX_DOORS_PER_WALL);

      rooms[room_editor_id].connections[other_room_editor_id] =
          random_direction;

      // Set opposite direction for the other room.
      Direction opposite_direction = get_opposite_direction(random_direction);
      rooms[other_room_editor_id].connections[room_editor_id] =
          opposite_direction;

      // Mark this pair as processed.
      processed_pairs.insert({room_editor_id, other_room_editor_id});
      processed_pairs.insert({other_room_editor_id, room_editor_id});
    }
  }

  // Debugging output.
  std::cout << "Adjacency List with Door Connections" << std::endl;
  for (int room_number = 0; room_number < (int)rooms.size(); room_number++) {
    EditorID room_id = std::to_string(room_number);
    std::unordered_map<EditorID, Direction> connections =
        rooms[room_id].connections;
    std::cout << "room " << room_number << ": ";
    for (auto const& pair : connections) {
      EditorID  other_room_id = pair.first;
      Direction direction     = pair.second;
      switch (direction) {
        case Direction::NORTH:
          std::cout << "(NORTH," << other_room_id << ") ";
          break;
        case Direction::EAST:
          std::cout << "(EAST," << other_room_id << ") ";
          break;
        case Direction::SOUTH:
          std::cout << "(SOUTH," << other_room_id << ") ";
          break;
        case Direction::WEST:
          std::cout << "(WEST," << other_room_id << ") ";
          break;
      }
    }
    std::cout << "\n";
  }
}

void LevelBuilder::randomize_connections() {
  std::mt19937 rng = getGlobalRNG();

  int  first_room_in_cluster = 0;
  uint normal_rooms;
  for (int cluster = 0; cluster < (int)ROOM_CLUSTERS.size(); cluster++) {
    int rooms_in_cluster = ROOM_CLUSTERS[cluster];
    normal_rooms         = rooms_in_cluster - 1;
    int max_connections  = normal_rooms - 1;

    // If there is only one room in this cluster for whatever reason, just
    // connect it and continue.
    if (rooms_in_cluster == 1) {
      // Mark the first room that begins a new difficulty area.
      first_room_in_cluster += ROOM_CLUSTERS[cluster];

      rooms[std::to_string(first_room_in_cluster)];
      continue;
    }

    // Generate a random path through this cluster of rooms, since we want our
    // level to be fully-connected.
    std::vector<int> random_path(rooms_in_cluster);
    for (int i = 0; i < rooms_in_cluster; ++i) {
      random_path[i] = first_room_in_cluster + i;
    }
    // Keep the first and last nodes fixed, so we can connect difficulty areas
    // via first_room_in_cluster and the elements of the input vector.
    std::shuffle(random_path.begin() + 1, random_path.end() - 1, rng);

    // Fill in the adjacency list for the path we just generated.
    // Also add it to the traversal path.
    EditorID room_id;
    for (int i = 0; i < (int)random_path.size(); i++) {
      int room = random_path[i];
      room_id  = std::to_string(room);

      if (i == 0) {
        rooms[room_id].connections[std::to_string(random_path[i + 1])];
      } else if (i == rooms_in_cluster - 1) {
        rooms[room_id].connections[std::to_string(random_path[i - 1])];
      } else {
        rooms[room_id].connections[std::to_string(random_path[i + 1])];
        rooms[room_id].connections[std::to_string(random_path[i - 1])];
      }

      traversal.push_back(room_id);
    }

    // Notice that we skip the boss room because we want to keep it a funnel
    // point that delineates area transitions.
    for (int i = 0; i < normal_rooms; i++) {
      int room = random_path[i];
      room_id  = std::to_string(room);

      // Naturally, skip any rooms that are already too full.
      if (rooms[room_id].connections.size() >= max_connections) {
        continue;
      }

      // Randomly connect this room to other ones.
      // Again, since the boss room should be sparse, decrement max_connections
      // by one.
      int num_connections =
          getRandInt(0, max_connections - rooms[room_id].connections.size() - 1);

      while (num_connections > 0) {
        int      other_room;
        EditorID other_room_id;

        do {
          other_room    = random_path[getRandInt(0, normal_rooms)];
          other_room_id = std::to_string(other_room);
          std::cout << other_room_id << std::endl;
        } while (
            // Discard this candidate other room if any of these are true:
            // 1. It is equal to the connectee room.
            // 2. It is already connected to the connectee room.
            // 3. It already has the maximum number of connections.
            other_room_id == room_id ||
            rooms[other_room_id].connections.count(room_id) > 0 ||
            rooms[other_room_id].connections.size() >= max_connections ||
            other_room == random_path.back());

        // Add the connection.
        rooms[room_id].connections[other_room_id];
        rooms[other_room_id].connections[room_id];

        // decrement num_connections
        num_connections--;
      }
    }

    // Mark the first room that begins a new difficulty area.
    first_room_in_cluster += ROOM_CLUSTERS[cluster];
  }

  // Connect all the boss rooms together that delineate a new difficulty area.
  // The boss rooms are guaranteed to have available connections.
  int boss_room;
  for (int i = 0; i < (int)MINIBOSS_ROOMS.size(); i++) {
    boss_room = MINIBOSS_ROOMS[i];
    rooms[std::to_string(boss_room)].connections[std::to_string(boss_room + 1)];
    rooms[std::to_string(boss_room + 1)].connections[std::to_string(boss_room)];
  }

  // Debug output.
  std::cout << "Graph:" << std::endl;
  int total_rooms = 0;
  for (int n : ROOM_CLUSTERS) {
    total_rooms += n;
  }
  for (int i = 0; i < total_rooms; i++) {
    std::cout << "room " << i << ": ";
    for (const auto& j : rooms[std::to_string(i)].connections) {
      std::cout << j.first << " ";
    }
    std::cout << "\n";
  }
}

Direction get_opposite_direction(Direction direction) {
  Direction opposite;
  switch (direction) {
    case Direction::NORTH:
      opposite = Direction::SOUTH;
      break;
    case Direction::EAST:
      opposite = Direction::WEST;
      break;
    case Direction::SOUTH:
      opposite = Direction::NORTH;
      break;
    case Direction::WEST:
      opposite = Direction::EAST;
      break;
  }
  return opposite;
};
