#pragma once

#include <vector>
#include <map>
#include <memory>
#include <unordered_map>

#include "room.hpp"
#include "hallway.hpp"

/**
 * The Following Define Standard Room IDs
 */

#define ROOM_ONE "room_1"
#define ROOM_TWO "room_2"

/**
 * The Following Define Standardized Units For Room Building
 *
 * NOTE: Rooms will have a standard 20 x 10 overall shape
 */

#define X_1U (window_width_px) / 21.3333f // One Room Building Unit in the X Direction
#define X_2U 2 * X_1U
#define X_3U 3 * X_1U
#define X_4U 4 * X_1U
#define X_5U 5 * X_1U
#define X_6U 6 * X_1U
#define X_7U 7 * X_1U
#define X_8U 8 * X_1U
#define X_9U 9 * X_1U
#define X_10U 10 * X_1U
#define X_11U 11 * X_1U
#define X_12U 12 * X_1U
#define X_13U 13 * X_1U
#define X_14U 14 * X_1U
#define X_15U 15 * X_1U
#define X_16U 16 * X_1U
#define X_17U 17 * X_1U
#define X_18U 18 * X_1U
#define X_19U 19 * X_1U
#define X_20U 20 * X_1U

#define Y_1U (window_height_px) / 12.f // One Room Building Unit in the Y Direction
#define Y_2U 2 * Y_1U
#define Y_3U 3 * Y_1U
#define Y_4U 4 * Y_1U
#define Y_5U 5 * Y_1U
#define Y_6U 6 * Y_1U
#define Y_7U 7 * Y_1U
#define Y_8U 8 * Y_1U
#define Y_9U 9 * Y_1U
#define Y_10U 10 * Y_1U

/**
 * A high-level OOP to ECS wrapper API that deals with level construction. Currently, it allows you to easily edit components such as
 * a room or hallway's walls, doors, and connections.
 */
class LevelBuilder
{
private:
    std::unordered_map<std::string, RoomBuilder> rooms;
    std::unordered_map<std::string, HallwayBuilder> hallways;

    void connect(Entity &connectee, Entity &connector);
    void print_pair(std::pair<std::string, Entity> pair);

public:
    LevelBuilder();

    /**
     * Returns the RoomBuilder for a room, making a new one if one under the given key does not exist.
     * @param s_id: the room's key.
     */
    RoomBuilder &room(std::string s_id);

    /**
     * Returns the HallwayBuilder for a hallway, making a new one if one under the given key does not exist.
     * @param s_id: the room's key.
     */
    HallwayBuilder &hallway(std::string s_id);

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
    LevelBuilder &connect_room_to_hallway(std::string r_id, std::string d1_id, std::string h_id, std::string d2_id);

    /**
     * Prints all rooms, hallways, (and their individual connections) inside a level in a sort-of human-readable manner for debugging purposes.
     */
    void print_rooms();
    void print_hallways();

    /**
     * @brief The Following Build Pre-Designed Rooms
     */
    void buildRoomOne();
    void buildRoomTwo();
};