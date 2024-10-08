#pragma once

#include <vector>
#include <map>
#include <memory>
#include <unordered_map>

#include "room.hpp"
#include "hallway.hpp"

/**
* A high-level OOP to ECS wrapper API that deals with level construction. Currently, it allows you to easily edit components such as 
* a room or hallway's walls, doors, and connections.
*/
class LevelBuilder {
    private:
        std::unordered_map<std::string, RoomBuilder> rooms;
        std::unordered_map<std::string, HallwayBuilder> hallways;

        void connect(Entity& connectee, Entity& connector);
        void print_pair(std::pair<std::string, Entity> pair);

    public:
        LevelBuilder();

        /**
        * Returns the RoomBuilder for a room, making a new one if one under the given key does not exist.
        * @param s_id: the room's key.
        */
        RoomBuilder& room(std::string s_id);

        /**
        * Returns the HallwayBuilder for a hallway, making a new one if one under the given key does not exist.
        * @param s_id: the room's key.
        */
        HallwayBuilder& hallway(std::string s_id);

        /**
        * Copies the shape and bounding box of one room to another. Does not copy that room's adjacencies.
        * @param s_id: the room's key.
        * @param copied_s_id: the room-to-be-copied's key.
        */
        RoomBuilder copy_room(std::string s_id, std::string copied_s_id);

        /**
        * Connects two rooms together through a hallway.
        * @param r1_id: the first room's key.
        * @param d1_id: the door in the first room's key.
        * @param h_id: the hallway's key.
        * @param d1_id: the door in the hallway's key.
        */
        LevelBuilder& connect_room_to_hallway(std::string r_id, std::string d1_id, std::string h_id, std::string d2_id);

        /**
        * Prints all rooms, hallways, (and their individual connections) inside a level in a sort-of human-readable manner for debugging purposes.
        */   
        void print_rooms();
        void print_hallways();
};