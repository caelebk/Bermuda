#pragma once

#include <vector>

#include "space.hpp"

struct Room : Entity {
    Room();
};

class RoomBuilder : public SpaceBuilder<Room> {
    private:
        bool is_in_room(vec2& position);
        vec2 rejection_sample();

    public:
        RoomBuilder();

        /**
        * Get a random position inside a room.
        * @param enemy: the enemy.
        */
        vec2 get_random_position();

        /**
        * Gets a list of vector representations of a room's walls.
        * @param enemy: the enemy.
        */
        // std::vector<Vector> get_walls();

        /**
        * Gets a list of vector representations of a room's doors.
        * @param enemy: the enemy.
        */
        // std::vector<Vector> get_doors();

        /**
        * Places an enemy in the room at a random position.
        * @param enemy: the enemy.
        */
        // void place_enemy_in_room_at_random_position(Entity enemy);
};