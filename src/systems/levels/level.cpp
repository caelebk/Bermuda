#include "level.hpp"

#include "common.hpp"
#include "room.hpp"

LevelBuilder::LevelBuilder(){};

RoomBuilder& LevelBuilder::room(std::string s_id) {
  return rooms[s_id];
};

HallwayBuilder& LevelBuilder::hallway(std::string s_id) {
  return hallways[s_id];
};

void LevelBuilder::connect(Entity& connectee, Entity& connector) {
  registry.adjacencies.get(connectee).neighbours.push_back(connector);
};

LevelBuilder& LevelBuilder::connect_room_to_hallway(std::string r_id,
                                                    std::string d1_id,
                                                    std::string h_id,
                                                    std::string d2_id) {
  RoomBuilder&    room    = rooms[r_id];
  HallwayBuilder& hallway = hallways[h_id];

  connect(room.entity, hallway.entity);
  connect(hallway.doors[d1_id], room.doors[d2_id]);
  connect(hallway.entity, room.entity);
  return *this;
};

RoomBuilder LevelBuilder::copy_room(std::string s_id, std::string copied_s_id) {
  room(s_id);
  registry.spaces.get(room(s_id).entity) =
      registry.spaces.get(room(copied_s_id).entity);
  registry.bounding_boxes.get(room(s_id).entity) =
      registry.bounding_boxes.get(room(copied_s_id).entity);
  return rooms[s_id];
};

void LevelBuilder::print_pair(std::pair<std::string, Entity> pair) {
  std::cout << "==================" << std::endl;
  std::cout << "  key:" << pair.first << std::endl;
  std::cout << "  id:" << pair.second << std::endl;
  Space& space = registry.spaces.get(pair.second);
  std::cout << "  walls: " << std::endl;
  for (auto& wall : space.walls) {
    Vector& vector = registry.vectors.get(wall);
    printf("    (%f, %f), (%f, %f):\n", vector.start.x, vector.start.y,
           vector.end.x, vector.end.y);
  }
  std::cout << "  doors:" << std::endl;
  for (auto& door : space.doors) {
    Vector& vector = registry.vectors.get(door);
    printf("    (%f, %f), (%f, %f):\n", vector.start.x, vector.start.y,
           vector.end.x, vector.end.y);
  }
  std::cout << "  connections:" << std::endl;
  Adjacency& adjacencies = registry.adjacencies.get(pair.second);
  for (auto& neighbour : adjacencies.neighbours) {
    std::cout << "    " << neighbour << std::endl;
  }
};

void LevelBuilder::print_rooms() {
  std::cout << "rooms:" << std::endl;
  for (auto pair : rooms) {
    print_pair(std::make_pair(pair.first, rooms[pair.first].entity));
  };
};

void LevelBuilder::print_hallways() {
  std::cout << "hallways:" << std::endl;
  for (auto pair : hallways) {
    print_pair(std::make_pair(pair.first, hallways[pair.first].entity));
  };
};

/********************************************************************************
 * @brief Build Pre-Designed Room 1 No Doors
 ********************************************************************************/
void LevelBuilder::buildRoomOne()  // TODO: REMOVE AFTER M1
{
  room(ROOM_ONE).up(Y_10U).right(X_14U).down(Y_5U).right(X_6U).down(Y_5U).left(
      X_20U);
};

/********************************************************************************
 * @brief Build Pre-Designed Room 2 No Doors
 ********************************************************************************/
void LevelBuilder::buildRoomTwo()  // TODO: REMOVE AFTER M1
{
  room(ROOM_TWO).up(Y_10U).right(X_20U).down(Y_10U).left(X_20U);
};

/********************************************************************************
 * @brief Build Pre-Designed Room 1
 ********************************************************************************/
// void LevelBuilder::buildRoomTwo()
// {
//     room(ROOM_ONE).up(Y_10U).right(X_6U).door("d1",
//     X_2U).right(X_6U).down(Y_5U).right(X_6U).down(Y_1U).door("d2",
//     Y_3U).down(Y_1U).left(X_20U);
// };

/********************************************************************************
 * @brief Build Pre-Designed Room 2
 ********************************************************************************/
// void LevelBuilder::buildRoomThree()
// {
//     room(ROOM_TWO).up(Y_10U).right(X_20U).down(Y_4U).door("d3",
//     Y_2U).down(Y_4U).left(X_20U);
// };