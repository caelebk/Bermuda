#include "level_build_system.hpp"
#include "level.hpp"

void level_builder_demo() {
    // All edits you make will go into the ECS.
    LevelBuilder level_builder = LevelBuilder();

    // You can create a room or hallway and start building its boundaries by 'drawing' it as a continuous line. Each unit here
    // is a magnitude in object space.
    level_builder.room("room_1").up(6).right(6).down(6).left(6);

    // You can copy the shape of rooms you have already built.
    level_builder.copy_room("copy_of_room_1", "room_1");

    // Doors can be added with the same chaining method, with the exception that you don't have to specify a direction for them, since they're
    // automatically inferred by the API. This example adds a room similar to room_1, but with doors in the middle of the top and bottom walls.
    level_builder.room("room_1_with_doors").up(6).right(2).door(2).right(2).down(6).left(2).door(2).left(2);

    // Hallways are built in exactly the same way. You can change the direction of the builder's 'cursor' by calling a
    // directional function without a magnitude. 
    level_builder.hallway("hallway_1").up(12).right().door(2).down(12).left().door(2);

    // You can connect rooms and hallways together.
    level_builder.copy_room("copy_of_room_1_with_doors", "room_1_with_doors");
    level_builder.connect_rooms("room_1_with_doors", "hallway_1", "copy_of_room_1_with_doors");

    // You can also get a random position in a room. Whether or not this method will remain private or public in the future depends on
    // some design decisions. See "place_enemy_in_room_at_random_position()" in room.hpp and room.cpp to see what I mean.
    Position position = level_builder.room("room_1_with_doors").get_random_position();
    printf("random position: (%i,%i)\n", position.x, position.y);

    // And yes, this works on non-square rectilinear polygons as well.
    level_builder.room("complex").up(10).right(4).door(2).right(4).down(5).right(5).down(1).door(3).down(1).left(15);
    // This room looks like this: https://www.desmos.com/calculator/dmbdboji9g
    for (int i = 0; i < 50; i++) {
        Position position = level_builder.room("complex").get_random_position();
        // None of these should be outside the boundary of the room.
        printf("random position: (%i,%i)\n", position.x, position.y);
    }

    level_builder.print_rooms();
    level_builder.print_hallways();
};