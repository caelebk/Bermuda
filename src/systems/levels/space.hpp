#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "tiny_ecs_registry.hpp"

#define ROOM_ORIGIN_POS                                                        \
  vec2(1.75f * (window_width_px) / 22.f,                                       \
       0.25f * (window_height_px) /                                            \
           12.f) // Position of Room Origin (Top-Left Corner)
#define WALL_THICKNESS 16.f

// TODO:
// If we go ahead with 86'ing hallways, this has no reason to be a template.
template <typename T> class SpaceBuilder {
protected:
  std::function<vec2(int)> direction; // the last direction of a SpaceBuilder boundary construction call.
  vec2 pointer; // the absolute position of where the 'cursor' is, i.e it's position is that of the last wall added to this SpaceBuilder.

  virtual void new_entity(T entity);

  virtual void update_bounding_box(Vector &vector);
  virtual Entity make_boundary(int magnitude);
  virtual SpaceBuilder &add_wall(int magnitude);

  virtual vec2 get_updated_up_position(int magnitude);
  virtual vec2 get_updated_down_position(int magnitude);
  virtual vec2 get_updated_right_position(int magnitude);
  virtual vec2 get_updated_left_position(int magnitude);

  vec2 rejection_sample();

public:
  T entity; // the space under construction.
  std::unordered_map<std::string, Entity> doors; // the doors belonging to the space.
  SpaceBuilder();

  virtual SpaceBuilder &up(int magnitude = 0);
  virtual SpaceBuilder &down(int magnitude = 0);
  virtual SpaceBuilder &left(int magnitude = 0);
  virtual SpaceBuilder &right(int magnitude = 0);
  virtual SpaceBuilder &door(std::string s_id, int magnitude = 0);

  bool is_in_room(vec2 &position);
  vec2 get_random_position();
};

template <typename T>
SpaceBuilder<T>::SpaceBuilder() : direction(), pointer({0, 0}) {}

template <typename T> void SpaceBuilder<T>::new_entity(T entity) {
  this->entity = entity;
  registry.spaces.emplace(entity);
  registry.bounding_boxes.emplace(entity);
}

template <typename T>
void SpaceBuilder<T>::update_bounding_box(Vector &vector) {
  SpaceBoundingBox &bounding_box = registry.bounding_boxes.get(entity);
  bounding_box.minimum_x = std::min(bounding_box.minimum_x, vector.end.x);
  bounding_box.maximum_x = std::max(bounding_box.maximum_x, vector.end.x);
  bounding_box.minimum_y = std::min(bounding_box.minimum_y, vector.end.y);
  bounding_box.maximum_y = std::max(bounding_box.maximum_y, vector.end.y);
};

template <typename T>
Entity SpaceBuilder<T>::make_boundary(int magnitude) {
    Entity boundary = Entity();
    vec2 endpoint = direction(magnitude);

    // Add the boundary (wall or door) to the appropriate component containers.
    Vector vector = Vector(pointer, endpoint);
    registry.vectors.insert(boundary, vector);

    Space &space = registry.spaces.get(entity);
    space.boundaries.push_back(boundary);

    // Update the space's bounding box.
    update_bounding_box(vector);

    // Update the last direction of the builder.
    pointer = endpoint;

    return boundary;
}

template <typename T>
SpaceBuilder<T> &SpaceBuilder<T>::add_wall(int magnitude) {
  if (magnitude != 0) {
    Space &space = registry.spaces.get(entity);
    Entity boundary = make_boundary(magnitude);
    space.walls.push_back(boundary);
  }
  return *this;
};

template <typename T>
SpaceBuilder<T> &SpaceBuilder<T>::door(std::string s_id, int magnitude) {
  Space &space = registry.spaces.get(entity);
  Entity boundary = make_boundary(magnitude);

  // Add the door to the space, keyed by string.
  doors[s_id] = boundary;

  space.doors.push_back(boundary);
  return *this;
}

template <typename T>
vec2 SpaceBuilder<T>::get_updated_up_position(int magnitude) {
  return {pointer.x, pointer.y + magnitude};
}

template <typename T>
vec2 SpaceBuilder<T>::get_updated_down_position(int magnitude) {
  return {pointer.x, pointer.y - magnitude};
}

template <typename T>
vec2 SpaceBuilder<T>::get_updated_right_position(int magnitude) {
  return {pointer.x + magnitude, pointer.y};
}

template <typename T>
vec2 SpaceBuilder<T>::get_updated_left_position(int magnitude) {
  return {pointer.x - magnitude, pointer.y};
}

template <typename T> SpaceBuilder<T> &SpaceBuilder<T>::up(int magnitude) {
  direction = [this](int magnitude) -> vec2 {
    return this->get_updated_up_position(magnitude);
  };
  return add_wall(magnitude);
}

template <typename T> SpaceBuilder<T> &SpaceBuilder<T>::down(int magnitude) {
  direction = [this](int magnitude) -> vec2 {
    return this->get_updated_down_position(magnitude);
  };
  return add_wall(magnitude);
}

template <typename T> SpaceBuilder<T> &SpaceBuilder<T>::left(int magnitude) {
  direction = [this](int magnitude) -> vec2 {
    return this->get_updated_left_position(magnitude);
  };
  return add_wall(magnitude);
}

template <typename T> SpaceBuilder<T> &SpaceBuilder<T>::right(int magnitude) {
  direction = [this](int magnitude) -> vec2 {
    return this->get_updated_right_position(magnitude);
  };
  return add_wall(magnitude);
}

template <typename T> bool SpaceBuilder<T>::is_in_room(vec2 &position) {
  // All well-formed rooms are rectilinear polygons. Therefore, if a point is
  // inside a room, it must be bounded in all four directions by at least one
  // wall.
  bool left = false;
  bool right = false;
  bool up = false;
  bool down = false;

  for (Entity &entity : registry.spaces.get(entity).boundaries) {
    Vector &vector = registry.vectors.get(entity);
    if ((!down || !up) && vector.start.y == vector.end.y) {
      if (position.x >
              (std::min(vector.start.x, vector.end.x) + WALL_THICKNESS) &&
          position.x <
              (std::max(vector.start.x, vector.end.x) - WALL_THICKNESS)) {
        if (position.y > vector.start.y) {
          down = true;
        } else {
          up = true;
        }
      }
    }

    if ((!left || !right) && vector.start.x == vector.end.x) {
      if (position.y > (std::min(vector.start.y, vector.end.y) + WALL_THICKNESS) &&
          position.y < (std::max(vector.start.y, vector.end.y) - WALL_THICKNESS)) {
        if (position.x > vector.start.x) {
          left = true;
        } else {
          right = true;
        }
      }
    }
  }

  return left && right && up && down;
};

template <typename T> vec2 SpaceBuilder<T>::rejection_sample() {
  SpaceBoundingBox &box = registry.bounding_boxes.get(entity);
  std::random_device rd;
  std::mt19937 generate(rd());
  // We generate random coordinates inside the bounding box. Theoretically, this
  // could be slow if the bounding box's area is far larger than a room; think
  // of a case like an 'L'-shaped room. Since randomized levels are only
  // generated once, and we can always code level generation so that our rooms
  // are 'decently' rectangular however, it should suffice.
  std::uniform_int_distribution<> random_x_generator(box.minimum_x,
                                                     box.maximum_x);
  std::uniform_int_distribution<> random_y_generator(box.minimum_y,
                                                     box.maximum_y);
  while (true) {
    vec2 position = {random_x_generator(generate),
                     random_y_generator(generate)};
    if (is_in_room(position)) {
      position += ROOM_ORIGIN_POS;
      return position;
    }
  };
};

template <typename T> vec2 SpaceBuilder<T>::get_random_position() {
  return rejection_sample();
}
