#include <stdio.h>
#include <random>
#include <chrono>
#include <thread>

#include "room.hpp"

#include "tiny_ecs_registry.hpp"

Room::Room(): Entity() {};

RoomBuilder::RoomBuilder(): SpaceBuilder<Room>() {
    new_entity(Room());
};

/*
void place_enemy_in_room_at_random_position(Entity enemy) {
    // If we don't want a room to know about its enemies, we can just have the enemy creation module call get get_random_position(),
    // and edit the registry however it wants, but if we want the rooms to know about its enemies, we probably want to go through a specialized function.
    registry.inside.insert({enemy, entity});
}
*/

bool RoomBuilder::is_in_room(Position& position, BoundingBox& box) {
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;

    for (Entity& entity : registry.spaces.get(entity).boundaries) {
        Vector& vector = registry.vectors.get(entity);
        if ((!down or !up) and vector.start.y == vector.end.y) {
            if (position.x > std::min(vector.start.x, vector.end.x) and position.x < std::max(vector.start.x, vector.end.x)) {
                if (position.y > vector.start.y) {
                    down = true;
                } else {
                    up = true;
                }
            }
        }

        if ((!left or !right) and vector.start.x == vector.end.x) {
            if (position.y > std::min(vector.start.y, vector.end.y) and position.y < std::max(vector.start.y, vector.end.y)) {
                if (position.x > vector.start.x) {
                    left = true;
                } else {
                    right = true;
                }
            }
        }
    }

    return left == 1 && right == 1 && up == 1 && down == 1;
};

Position RoomBuilder::rejection_sample() {
    BoundingBox& box = registry.bounding_boxes.get(entity);
    std::random_device random; 
    std::mt19937 generate(random()); 
    std::uniform_int_distribution<> random_x_generator(box.minimum_x, box.maximum_x); 
    std::uniform_int_distribution<> random_y_generator(box.minimum_y, box.maximum_y); 
    while (true) {
        Position position = Position(random_x_generator(generate), random_y_generator(generate));
        if (is_in_room(position, box)) {
            return position;
        }
    };
};

Position RoomBuilder::get_random_position() {
    return rejection_sample();

}