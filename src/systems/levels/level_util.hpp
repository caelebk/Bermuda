#pragma once

#include <vector>
#include <functional>

#include "render_system.hpp"

using EditorID = std::string;

// A group of individual spawning functions.
using SpawnFunction = std::function<Entity(RenderSystem *r, vec2 p, bool b)>;

enum class Objective {
    NONE,
    RED_KEY,
    BLUE_KEY,
    YELLOW_KEY,
    PRESSURE_PLATE,
    BOSS,
};

struct SpawnFunctionWrapper {
    SpawnFunction spawn_function; // the actual spawning function to use.
    float probability; // the probability that this spawn will be executed again on room entry, for some randomness. 0.0f only spawns once.
    int pack_size; // the pack size to spawn. If 0, the spawn will be treated as normal.

    SpawnFunctionWrapper(SpawnFunction spawn_function, float probability, int pack_size):
     spawn_function(spawn_function), probability(probability), pack_size(pack_size) {};
};