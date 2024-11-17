#pragma once

#include <vector>

#include "level_util.hpp"
#include "level_spawn.hpp"

#define DOOR_SIZE 1
#define MAX_DOORS_PER_WALL 2
 
// The keys that can actually spawn during a level.
const std::vector<INVENTORY> KEYS = {INVENTORY::RED_KEY, INVENTORY::BLUE_KEY, INVENTORY::YELLOW_KEY};
// The spawn functions for the keys.
const std::vector<SpawnFunctionGroup> KEY_SPAWN_FUNCTIONS = {RED_KEY_SPAWN, BLUE_KEY_SPAWN, YELLOW_KEY_SPAWN};
// The probability of a door spawning as locked via any available key, as a percentage.
const int LOCKED_DOOR_PROBABILITY = 75;

const EditorID STARTING_ROOM = "0";
const EditorID TUTORIAL_ROOM = "0";
const EditorID FINAL_BOSS_ROOM = "15";

// Each element of this vector is how many rooms are in a difficulty cluster; i.e the first (tutorial) level has one room,
// the next has five, etc.
const std::vector<int> ROOM_CLUSTERS = {1, 5, 5, 5}; // 16 rooms total.
// This vector controls index-wise the spawn function groups of the levels you defined in the above line, i.e the first (tutorial level) 
// spawns nothing, the next spawns LVL_1 enemies, etc. Must have size == to ROOM_CLUSTERS.size().
const std::vector<SpawnFunctionGroup> ROOM_CLUSTER_SPAWN_FUNCTION_GROUPS = {EMPTY, LVL_1_RAND_POS, LVL_2_RAND_POS, LVL_3_RAND_POS};
const std::vector<SpawnFunctionGroup> ROOM_CLUSTER_PACK_SPAWN_FUNCTION_GROUPS = {EMPTY, LVL_1_PACKS, LVL_2_PACKS, EMPTY};

const std::vector<int> MINIBOSS_ROOMS = {0, 5, 10};
// This vector controls index-wise the spawn function groups of miniboss rooms you defined in the above line, i.e the first (tutorial level) 
// spawns the tutorial boss, the next spawns the giant crab, etc. Must have size == to MINIBOSS_ROOMS.size().
const std::vector<SpawnFunctionGroup> MINIBOSS_SPAWN_FUNCTION_GROUPS = {TUTORIAL_JELLYFISH_MINIBOSS, CRAB_MINIBOSS, SHARKMAN_MINIBOSS};
