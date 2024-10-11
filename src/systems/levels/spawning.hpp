#pragma once

#include "render_system.hpp"
#include <functional>
#include <glm/ext/vector_float2.hpp>
#include <initializer_list>
#include <random>
#include <vector>

#include "space.hpp"

template <typename T>
void execute_config_rand(
    const std::initializer_list<std::function<Entity(RenderSystem *r, vec2 p)>>
        &funcs,
    SpaceBuilder<T> &space_builder, RenderSystem *renderer) {
  for (const auto &func : funcs) {
  vec2 loc = space_builder.get_random_position();
    func(renderer, loc); // Call each function
  }
}

void execute_config_fixed(const std::vector<std::function<void()>> &funcs);
void execute_config_fixed_rand(float chance,
                               const std::vector<std::function<void()>> &funcs);
