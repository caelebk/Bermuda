#pragma once

#include <vector>

#include "space.hpp"

struct Room : Entity {
    Room();
};

class RoomBuilder : public SpaceBuilder<Room> {
    public:
        RoomBuilder();

        /**
        * Places an enemy in the room at a random position.
        * @param enemy: the enemy.
        */
        // void place_enemy_in_room_at_random_position(Entity enemy);
};