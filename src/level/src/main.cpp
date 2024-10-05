// #include <memory>
// #include <iostream>
// #include <stdio.h> 

// #include "../include/level.hpp"

// void test_three_rooms_vertically() {
//     LevelBuilder* level_builder = new LevelBuilder();

//     level_builder->room("room_1")->up(10)->right(10)->down(10)->left(4)->door(2)->left(4)->done();
//     level_builder->new_hallway("hallway_1_2")->up(10)->right()->door(2)->down(10)->left()->door(2)->done();
//     level_builder->room("room_2")->up(10)->right(4)->door(2)->right(4)->down(10)->left(4)->door(2)->left(4)->done();
//     level_builder->new_hallway("hallway_2_3")->up(10)->right(4)->door(2)->right(4)->down(10)->left(4)->door(2)->left(4)->done();
//     level_builder->room("room_3")->up(10)->right(4)->door(2)->right(4)->down(10)->left(10)->done();

//     level_builder->connect_rooms("room_1", "hallway_1_2", "room_2");
//     level_builder->connect_rooms("room_2", "hallway_2_3", "room_3");

//     level_builder->level->print();
// };

// int main(int argc, char *argv[]) {
//     test_three_rooms_vertically();
//     return 1;
// }