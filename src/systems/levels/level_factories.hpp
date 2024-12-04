#pragma once

#include <vector>

#include "level_spawn.hpp"
#include "level_util.hpp"

#define DOOR_SIZE 1
#define MAX_DOORS_PER_WALL 2

// The keys that can actually spawn during a level.
const std::vector<Objective> KEYS = {Objective::RED_KEY, Objective::BLUE_KEY, Objective::YELLOW_KEY};
// The spawn functions for the keys.
const std::vector<SpawnFunctionWrapper> KEY_SPAWN_WRAPPERS = {RED_KEY_SPAWN, BLUE_KEY_SPAWN, YELLOW_KEY_SPAWN};
// The probability of a door spawning as locked via any unlock condition.
// const int LOCKED_DOOR_PROBABILITY = 50;
// The probability of a pressure plate possibly spawning in a room.
const int PRESSURE_PLATE_PROBABILITY = 100;

const EditorID STARTING_ROOM   = "0";
const EditorID TUTORIAL_ROOM   = "0";
const EditorID FINAL_BOSS_ROOM = "15";

// Each element of this vector is how many rooms are in a difficulty cluster; i.e the first (tutorial) level has one room,
// the next has five, etc.
const std::vector<int> ROOM_CLUSTERS = {1, 5, 5, 5}; // 16 rooms total.
// This vector controls index-wise the spawn function groups of the levels you defined in the above line, i.e the first (tutorial level) 
// spawns nothing, the next spawns LVL_1 enemies, etc. Must have size == to ROOM_CLUSTERS.size().
const std::vector<std::vector<SpawnFunctionWrapper>> ROOM_SPAWN_WRAPPERS = {EMPTY, LVL_1, LVL_2, LVL_3};

const std::vector<int> MINIBOSS_ROOMS = {0, 5, 10};
// This vector controls index-wise the spawn function groups of miniboss rooms you defined in the above line, i.e the first (tutorial level) 
// spawns the tutorial boss, the next spawns the giant crab, etc. Must have size == to MINIBOSS_ROOMS.size().
const std::vector<std::vector<SpawnFunctionWrapper>> MINIBOSS_SPAWN_WRAPPERS = {TUTORIAL_JELLYFISH_MINIBOSS, CRAB_MINIBOSS, SHARKMAN_MINIBOSS};
const std::vector<std::vector<SpawnFunctionWrapper>> FINAL_BOSS_SPAWN_WRAPPER = {FINAL_BOSS};
