#pragma once

#include <vector>
#include "render_system.hpp"
#include <functional>
#include <glm/ext/vector_float2.hpp>
#include <random>

void execute_config(const std::vector<std::function<void()>> &funcs);
void execute_config_rand(float chance,
                         const std::vector<std::function<void()>> &funcs);
