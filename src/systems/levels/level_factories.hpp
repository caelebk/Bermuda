#pragma once

#include <vector>

#include "level_spawn.hpp"

using SpawnFunction = std::initializer_list<std::function<Entity(RenderSystem *r, vec2 p, bool b)>>;
#define STARTING_ROOM_EDITOR_ID "0"

const int TUTORIAL_ROOM = 0;

// Each element of this vector is how many rooms are in a difficulty cluster; i.e the first (tutorial) level has one room,
// the next has five, etc.
const std::vector<int> ROOM_CLUSTERS = {1, 5, 5, 5}; // 16 rooms total.
// This vector controls index-wise the spawn functions of the levels you defined in the above line, i.e the first (tutorial level) 
// spawns nothing, the next spawns LVL_1 enemies, etc. Must have size == to ROOM_CLUSTERS.size().
const std::vector<SpawnFunction> ROOM_CLUSTER_SPAWN_FUNCTIONS = {EMPTY, LVL_1_RAND_POS, LVL_2_RAND_POS, LVL_3_RAND_POS};

const std::vector<int> MINIBOSS_ROOMS = {0, 5, 10};
// This vector controls index-wise the spawn functions of miniboss rooms you defined in the above line, i.e the first (tutorial level) 
// spawns the tutorial boss, the next spawns the giant crab, etc. Must have size == to MINIBOSS_ROOMS.size().
const std::vector<SpawnFunction> MINIBOSS_SPAWN_FUNCTIONS = {TUTORIAL_JELLYFISH_MINIBOSS, CRAB_MINIBOSS, SHARKMAN_MINIBOSS};

const int FINAL_BOSS_ROOM = 15;