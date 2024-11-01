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

#define MAX_X_UNITS 20
#define MAX_Y_UNITS 10

#define DOOR_SCALAR 2

#define X_1U (window_width_px) / 22.f // One Room Building Unit in the X Direction
#define X_2U 2.f * X_1U
#define X_3U 3.f * X_1U
#define X_4U 4.f * X_1U
#define X_5U 5.f * X_1U
#define X_6U 6.f * X_1U
#define X_7U 7.f * X_1U
#define X_8U 8.f * X_1U
#define X_9U 9.f * X_1U
#define X_10U 10.f * X_1U
#define X_11U 11.f * X_1U
#define X_12U 12.f * X_1U
#define X_13U 13.f * X_1U
#define X_14U 14.f * X_1U
#define X_15U 15.f * X_1U
#define X_16U 16.f * X_1U
#define X_17U 17.f * X_1U
#define X_18U 18.f * X_1U
#define X_19U 19.f * X_1U
#define X_20U 20.f * X_1U

#define Y_1U (window_height_px) / 12.f // One Room Building Unit in the Y Direction
#define Y_2U 2.f * Y_1U
#define Y_3U 3.f * Y_1U
#define Y_4U 4.f * Y_1U
#define Y_5U 5.f * Y_1U
#define Y_6U 6.f * Y_1U
#define Y_7U 7.f * Y_1U
#define Y_8U 8.f * Y_1U
#define Y_9U 9.f * Y_1U
#define Y_10U 10.f * Y_1U

// An enumeration for programatically defining directions.
enum Direction {
  NORTH = 0,
  EAST = 1,
  SOUTH = 2,
  WEST = 3
};

int get_random(int x, int y);
Direction get_opposite_direction(Direction direction);

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

    std::vector<int> get_random_door_positions(Direction direction, std::unordered_map<int, Direction>& directed_adjacencies, int min, int max);
    void build_wall_with_doors(Direction direction,
                              std::vector<int> other_rooms,
                              RoomBuilder& room, std::vector<int> door_positions, 
                              int max_units, int unit_size,
                              std::function<void(int)> draw_segment,
                              std::function<void(std::string, int)> draw_door);

    std::unordered_map<int, std::set<int>> generate_random_graph(std::vector<int>& rooms, std::vector<int>& adjacency_list);
    std::unordered_map<int, std::unordered_map<int, Direction>> generate_random_graph_doors(std::unordered_map<int, std::set<int>>& adjacency_list);
    void randomize_room_shapes(std::unordered_map<int, std::unordered_map<int, Direction>>& adjacency_list);
    void connect_doors(std::unordered_map<int, std::unordered_map<int, Direction>>& adjacency_list);
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
     * Connects two rooms together..
     * @param r1_id: the first room's key.
     * @param d1_id: the door in the first room's key.
     * @param h_id: the second room's key.
     * @param d1_id: the door in the second room's key.
     */
    LevelBuilder &connect_rooms(std::string r_id, std::string d1_id, std::string r2_id, std::string d2_id);

    /**
     * Generates a random level with a given number of rooms and a randomized number of hallways. Each element of the input represents a difficulty
     * cluster, i.e {7, 5, 3} means 7 easy rooms, 5 medium rooms, and 3 hard rooms. 
     * Each cluster has exactly one connection between them.
     */
    void generate_random_level(std::vector<int> rooms, std::vector<int> densities);


    /**
     * Prints all rooms, hallways, (and their individual connections) inside a level in a sort-of human-readable manner for debugging purposes.
     */
    void print_rooms();
    void print_hallways();

    void buildRoomOne();
};