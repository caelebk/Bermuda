#pragma once

#include <vector>
#include <functional>

#include "render_system.hpp"

using EditorID = std::string;

// A group of individual spawning functions.
using SpawnFunction = std::function<Entity(RenderSystem *r, vec2 p, bool b)>;
using SpawnFunctionGroup = std::vector<std::function<Entity(RenderSystem *r, vec2 p, bool b)>>;