#include <iostream>
#include <stdio.h>

#include "level.hpp"
#include "room.hpp"

#include "tiny_ecs_registry.hpp"

LevelBuilder::LevelBuilder() {};

RoomBuilder& LevelBuilder::room(std::string s_id) {
    return rooms[s_id];
};

HallwayBuilder& LevelBuilder::hallway(std::string s_id) {
    return hallways[s_id];
};

LevelBuilder& LevelBuilder::connect_rooms(std::string room_1_s_id, std::string hallway_s_id, std::string room_2_s_id) {
    rooms[room_1_s_id].connect(hallways[hallway_s_id].entity);
    hallways[hallway_s_id].connect(rooms[room_1_s_id].entity);
    hallways[hallway_s_id].connect(rooms[room_2_s_id].entity);
    rooms[room_2_s_id].connect(hallways[hallway_s_id].entity);
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
        printf("    (%i, %i), (%i, %i):\n", vector.start.x, vector.start.y, vector.end.x, vector.end.y);
    }
    std::cout << "  doors:" << std::endl;
    for (auto& door : space.doors) {
        Vector& vector = registry.vectors.get(door);
        printf("    (%i, %i), (%i, %i):\n", vector.start.x, vector.start.y, vector.end.x, vector.end.y);
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