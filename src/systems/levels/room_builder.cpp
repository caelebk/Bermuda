#include <iostream>

#include "level_factories.hpp"
#include "random.hpp"
#include "room_builder.hpp"
#include "spawning.hpp"

RoomBuilder::RoomBuilder(): pointer({0, 0}) {
  this->entity = entity;
  registry.spaces.emplace(entity);
  registry.bounding_boxes.emplace(entity);
}

void RoomBuilder::update_bounding_box(Vector &vector) {
  SpaceBoundingBox &bounding_box = registry.bounding_boxes.get(entity);
  bounding_box.minimum_x = std::min(bounding_box.minimum_x, vector.end.x);
  bounding_box.maximum_x = std::max(bounding_box.maximum_x, vector.end.x);
  bounding_box.minimum_y = std::min(bounding_box.minimum_y, vector.end.y);
  bounding_box.maximum_y = std::max(bounding_box.maximum_y, vector.end.y);
};

Entity RoomBuilder::make_boundary(int magnitude) {
    Entity boundary = Entity();
    vec2 endpoint = direction(magnitude);

    // Add the boundary (wall or door) to the appropriate component containers.
    Vector vector = Vector(pointer, endpoint);
    registry.vectors.insert(boundary, vector);
    float vector_width = abs(vector.end.x - vector.start.x);
    float vector_height  = abs(vector.end.y - vector.start.y);
    float vector_pos_x = (vector.end.x + vector.start.x) / 2;
    float vector_pos_y = (vector.end.y + vector.start.y) / 2;

    vec2 bounding_box;
    if (vector_width <= 0) {
      bounding_box = vec2(WALL_THICKNESS, vector_height + WALL_THICKNESS);
    } else if (vector_height <= 0) {
      bounding_box = vec2(vector_width + WALL_THICKNESS, WALL_THICKNESS);
    }
    vec2 position = vec2(vector_pos_x + ROOM_ORIGIN_POS.x,
                        vector_pos_y + ROOM_ORIGIN_POS.y);

    // Setting initial position values
    Position& position_component = registry.positions.emplace(boundary);
    position_component.position  = position;
    position_component.angle     = 0.f;
    position_component.scale     = bounding_box;

    Space &space = registry.spaces.get(entity);
    space.boundaries.push_back(boundary);

    // Update the space's bounding box.
    update_bounding_box(vector);

    // Update the last direction of the builder.
    pointer = endpoint;

    return boundary;
}

RoomBuilder& RoomBuilder::add_wall(int magnitude) {
  if (magnitude != 0) {
    Space &space = registry.spaces.get(entity);
    Entity boundary = make_boundary(magnitude);
    space.walls.push_back(boundary);
  }
  return *this;
};

RoomBuilder& RoomBuilder::door(EditorID s_id, int magnitude) {
  Space &space = registry.spaces.get(entity);
  Entity boundary = make_boundary(magnitude);

  // Add the door to the space, keyed by string.
  doors[s_id] = boundary;

  space.doors.push_back(boundary);
  return *this;
}

vec2 RoomBuilder::get_updated_up_position(int magnitude) {
  return {pointer.x, pointer.y + magnitude};
}

vec2 RoomBuilder::get_updated_down_position(int magnitude) {
  return {pointer.x, pointer.y - magnitude};
}

vec2 RoomBuilder::get_updated_right_position(int magnitude) {
  return {pointer.x + magnitude, pointer.y};
}

vec2 RoomBuilder::get_updated_left_position(int magnitude) {
  return {pointer.x - magnitude, pointer.y};
}

void RoomBuilder::respawn(RenderSystem* renderer) {
  while (saved_entities.size() > 0) {
    EntitySave es = saved_entities.back();
    es.respawn(renderer);
    saved_entities.pop_back();
  }
}

RoomBuilder& RoomBuilder::up(int magnitude) {
  direction = [this](int magnitude) -> vec2 {
    return this->get_updated_up_position(magnitude);
  };
  return add_wall(magnitude);
}

RoomBuilder& RoomBuilder::down(int magnitude) {
  direction = [this](int magnitude) -> vec2 {
    return this->get_updated_down_position(magnitude);
  };
  return add_wall(magnitude);
}

RoomBuilder& RoomBuilder::left(int magnitude) {
  direction = [this](int magnitude) -> vec2 {
    return this->get_updated_left_position(magnitude);
  };
  return add_wall(magnitude);
}

RoomBuilder& RoomBuilder::right(int magnitude) {
  direction = [this](int magnitude) -> vec2 {
    return this->get_updated_right_position(magnitude);
  };
  return add_wall(magnitude);
}

std::vector<EditorID> RoomBuilder::get_connections_with_direction(Direction direction) {
  std::vector<EditorID> connected_rooms;

  for (const auto& pair : connections) {
    EditorID  other_room_id  = pair.first;
    Direction pair_direction = pair.second;
    if (direction == pair_direction) {
      connected_rooms.push_back(other_room_id);
    }
  }

  return connected_rooms;
}

bool RoomBuilder::is_in_room(vec2& position) {
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

vec2 RoomBuilder::rejection_sample() {
  SpaceBoundingBox &box = registry.bounding_boxes.get(entity);
  // We generate random coordinates inside the bounding box. Theoretically, this
  // could be slow if the bounding box's area is far larger than a room; think
  // of a case like an 'L'-shaped room. Since randomized levels are only
  // generated once, and we can always code level generation so that our rooms
  // are 'decently' rectangular however, it should suffice.
  while (true) {
    vec2 position = {randomFloat(box.minimum_x, box.maximum_x),
                     randomFloat(box.minimum_y, box.maximum_y)};
    if (is_in_room(position)) {
      position += ROOM_ORIGIN_POS;
      return position;
    }
  };
};

void RoomBuilder::spawn(RenderSystem* renderer) {
  if (has_entered) {
    respawn(renderer);
  } else {
    for (const auto& spawn_wrapper : spawn_wrappers) {
      if (spawn_wrapper.pack_size == 0) { // if the spawn wrapper is not a pack...
        // We have to cast these to vectors for the spawn function signatures.
        execute_config_rand_chance({spawn_wrapper.spawn_function}, *this, renderer, spawn_wrapper.probability);
      } else {
        execute_pack_spawning({spawn_wrapper.spawn_function}, *this, renderer, spawn_wrapper.pack_size);
      }
    }
  }
  has_entered = true;
}

vec2 RoomBuilder::get_random_position() {
  return rejection_sample();
}
