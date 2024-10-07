#include "level_build_system.hpp"
#include "level.hpp"

void level_builder_demo() {
    std::cout << "level_builder_demo" << std::endl;
    // All edits you make will go into the ECS.
    LevelBuilder level_builder = LevelBuilder();

    // You can create a room or hallway and start building its boundaries by 'drawing' it as a continuous line. Each unit here
    // is a magnitude in object space.
    level_builder.room("room_1").up(6).right(6).down(6).left(6);

    // You can copy the shape of rooms you have already built.
    level_builder.copy_room("copy_of_room_1", "room_1");

    // Doors can be added with the same chaining method, with the exception that you don't have to specify a direction for them, since they're
    // automatically inferred by the API. Give each door an id so that we can connect them later. Door ids are scoped to each room, not each level!
    level_builder.room("room_1_with_doors").up(6).right(6).down(6).left(2).door("s", 2).left(2);

    // Hallways are built in exactly the same way. You can change the direction of the builder's 'cursor' by calling a
    // directional function without a magnitude. 
    level_builder.hallway("hallway_1").up(12).right().door("n", 2).down(12).left().door("s", 2);

    // You can connect rooms and hallways together. Make sure to specify which doors lead where.
    level_builder.room("room_2").up(6).right(2).door("n", 2).right(2).down(6).left(6);
    level_builder.connect_room_to_hallway("room_1_with_doors", "s", "hallway_1", "n");
    level_builder.connect_room_to_hallway("room_2", "n", "hallway_1", "s");

    // You can also get a random position in a room.
    vec2 position = level_builder.room("room_1_with_doors").get_random_position();
    printf("random position: (%f,%f)\n", position[0], position[1]);

    // And yes, this works on any non-square rectilinear space as well (rooms or hallways).
    level_builder.hallway("complex").up(10).right(4).door("n", 2).right(4).down(5).right(5).down(1).door("e", 3).down(1).left(15);
    // This room looks like this: https://www.desmos.com/calculator/dmbdboji9g
    for (int i = 0; i < 50; i++) {
        vec2 position = level_builder.hallway("complex").get_random_position();
        // None of these should be outside the boundary of the hallway.
        printf("random position: (%f,%f)\n", position[0], position[1]);
    }

    level_builder.print_rooms();
    level_builder.print_hallways();
};

void how_to_get_walls_and_doors_demo() {
    std::cout << "how_to_get_walls_and_doors_demo" << std::endl;
    LevelBuilder level_builder = LevelBuilder();
    level_builder.room("room_1").up(6).right(6).down(6).left(2).door("s", 2).left(2);

    // You can get all the room's wall components like this.
    std::vector<Vector> walls = level_builder.room("room_1").get_wall_vectors();
    std::cout << "walls: " << std::endl;
    for (auto& wall : walls) {
        printf("(%f, %f) (%f, %f)\n", wall.start[0], wall.start[1], wall.end[0], wall.end[1]);
    }

    // Ditto for doors.
    std::vector<Vector> doors = level_builder.room("room_1").get_door_vectors();
    std::cout << "doors: " << std::endl;
    for (auto& door : doors) {
        printf("(%f, %f) (%f, %f)\n", door.start[0], door.start[1], door.end[0], door.end[1]);
    }
}