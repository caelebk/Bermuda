#pragma once

#include <vector>

#include "types.hpp"

struct Room : Entity {
    Room();
};

class RoomBuilder : public SpaceBuilder<Room> {
    private:
        bool is_in_room(Position& position, BoundingBox& box);
        Position rejection_sample();

    public:
        RoomBuilder();

        /**
        * Get a random position inside a room.
        * @param enemy: the enemy.
        */
        Position get_random_position();

        /**
        * Places an enemy in the room at a random position.
        * @param enemy: the enemy.
        */
        // void place_enemy_in_room_at_random_position(Entity enemy);
};