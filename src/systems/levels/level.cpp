#include "common.hpp"
#include "level.hpp"
#include "room.hpp"

LevelBuilder::LevelBuilder() {};

RoomBuilder& LevelBuilder::room(std::string s_id) {
    return rooms[s_id];
};

HallwayBuilder& LevelBuilder::hallway(std::string s_id) {
    return hallways[s_id];
};

void LevelBuilder::connect(Entity& connectee, Entity& connector) {
    registry.adjacencies.get(connectee).neighbours.push_back(connector);
};

LevelBuilder& LevelBuilder::connect_room_to_hallway(std::string r_id, std::string d1_id, std::string h_id, std::string d2_id) {
    RoomBuilder& room = rooms[r_id];
    HallwayBuilder& hallway = hallways[h_id];

    connect(room.entity, hallway.entity);
    connect(hallway.doors[d1_id], room.doors[d2_id]);
    connect(hallway.entity, room.entity);
    return *this;
};

RoomBuilder LevelBuilder::copy_room(std::string s_id, std::string copied_s_id) {
    room(s_id);
    registry.spaces.get(room(s_id).entity) = registry.spaces.get(room(copied_s_id).entity);
    registry.bounding_boxes.get(room(s_id).entity) = registry.bounding_boxes.get(room(copied_s_id).entity);
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
        printf("    (%f, %f), (%f, %f):\n", vector.start[0], vector.start[1], vector.end[0], vector.end[1]);
    }
    std::cout << "  doors:" << std::endl;
    for (auto& door : space.doors) {
        Vector& vector = registry.vectors.get(door);
        printf("    (%f, %f), (%f, %f):\n", vector.start[0], vector.start[1], vector.end[0], vector.end[1]);
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