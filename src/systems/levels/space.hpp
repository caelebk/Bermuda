#pragma once

#include <functional>
#include <random>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

#include "tiny_ecs_registry.hpp"

struct Wall : Entity {
public:
    Wall();
};

struct Door : Wall {
public:
    Door();    
};

template <typename T>
class SpaceBuilder {
protected:
    std::function<vec2(int)> direction;
    vec2 pointer;

    virtual void new_entity(T entity);
    virtual SpaceBuilder& add_wall(int magnitude);

    virtual void update_bounding_box(Vector& vector);
    virtual vec2 get_updated_up_position(int magnitude);
    virtual vec2 get_updated_down_position(int magnitude);
    virtual vec2 get_updated_right_position(int magnitude);
    virtual vec2 get_updated_left_position(int magnitude);

    vec2 rejection_sample();
public:
    T entity;
    std::unordered_map<std::string, Entity> doors;
    SpaceBuilder();

    virtual SpaceBuilder& up(int magnitude = 0);
    virtual SpaceBuilder& down(int magnitude = 0);
    virtual SpaceBuilder& left(int magnitude = 0);
    virtual SpaceBuilder& right(int magnitude = 0);
    virtual SpaceBuilder& door(std::string s_id, int magnitude = 0);

    /**
    * Gets a list of vector representations of a room's walls.
    */
    std::vector<Vector> get_wall_vectors();
    /**
    * Gets a list of vector representations of a room's doors.
    */
    std::vector<Vector> get_door_vectors();

    bool is_in_room(vec2& position);
    /**
    * Get a random position inside a space.
    */
    vec2 get_random_position();

    // virtual SpaceBuilder* rotate_right(int rotations);
    // virtual SpaceBuilder* rotate_left(int rotations);
};

template <typename T>
SpaceBuilder<T>::SpaceBuilder() : direction(), pointer({0,0}) {}

template <typename T>
void SpaceBuilder<T>::new_entity(T entity) {
    this->entity = entity;
    registry.spaces.insert(entity, Space());
    registry.adjacencies.insert(entity, Adjacency());
    registry.bounding_boxes.insert(entity, SpaceBoundingBox());
}

template <typename T>
void SpaceBuilder<T>::update_bounding_box(Vector& vector) {
    SpaceBoundingBox& bounding_box = registry.bounding_boxes.get(entity);
    bounding_box.minimum_x = std::min(bounding_box.minimum_x, vector.end.x);
    bounding_box.maximum_x = std::max(bounding_box.maximum_x, vector.end.x);   
    bounding_box.minimum_y = std::min(bounding_box.minimum_y, vector.end.y);
    bounding_box.maximum_y = std::max(bounding_box.maximum_y, vector.end.y);  
};

template <typename T>
SpaceBuilder<T>& SpaceBuilder<T>::add_wall(int magnitude) {
    if (magnitude != 0) {
        vec2 endpoint = direction(magnitude);

        Entity wall = Wall();
        Vector vector = Vector(pointer, endpoint);
        registry.vectors.insert(wall, vector);

        Space& space = registry.spaces.get(entity);
        space.walls.push_back(wall);
        space.boundaries.push_back(wall);
        update_bounding_box(vector);
        pointer = endpoint;
    }
    return *this;    
};

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

template <typename T>
SpaceBuilder<T>& SpaceBuilder<T>::up(int magnitude) {
    direction = [this](int magnitude) -> vec2 { return this->get_updated_up_position(magnitude); };
    return add_wall(magnitude);
}

template <typename T>
SpaceBuilder<T>& SpaceBuilder<T>::down(int magnitude) {
    direction = [this](int magnitude) -> vec2 { return this->get_updated_down_position(magnitude); };
    return add_wall(magnitude);
}

template <typename T>
SpaceBuilder<T>& SpaceBuilder<T>::left(int magnitude) {
    direction = [this](int magnitude) -> vec2 { return this->get_updated_left_position(magnitude); };
    return add_wall(magnitude);
}

template <typename T>
SpaceBuilder<T>& SpaceBuilder<T>::right(int magnitude) {
    direction = [this](int magnitude) -> vec2 { return this->get_updated_right_position(magnitude); };
    return add_wall(magnitude);
}

template <typename T>
SpaceBuilder<T>& SpaceBuilder<T>::door(std::string s_id, int magnitude) {
    vec2 endpoint = direction(magnitude);

    Entity door = Door();
    doors[s_id] = door;
    Vector vector = Vector(pointer, endpoint);
    registry.adjacencies.insert(door, Adjacency());
    registry.vectors.insert(door, vector);

    Space& space = registry.spaces.get(entity);
    space.doors.push_back(door);
    space.boundaries.push_back(door);
    update_bounding_box(vector);
    pointer = endpoint;
    return *this;
}

template <typename T>
std::vector<Vector> SpaceBuilder<T>::get_wall_vectors() {
    std::vector<Vector> vectors;
    for (auto& wall : registry.spaces.get(entity).walls) {
        vectors.push_back(registry.vectors.get(wall));
    }
    return vectors;
}

template <typename T>
std::vector<Vector> SpaceBuilder<T>::get_door_vectors() {
    std::vector<Vector> vectors;
    for (auto& door : registry.spaces.get(entity).doors) {
        vectors.push_back(registry.vectors.get(door));
    }
    return vectors;
}

template <typename T>
bool SpaceBuilder<T>::is_in_room(vec2& position) {
    // All well-formed rooms are rectilinear polygons. Therefore, if a point is inside a room, it must be bounded in all four
    // directions by at least one wall.
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;

    
    for (Entity& entity : registry.spaces.get(entity).boundaries) {
        Vector& vector = registry.vectors.get(entity);
        if ((!down or !up) and vector.start.y == vector.end.y) {
            if (position.x > std::min(vector.start.x, vector.end.x) and position.x < std::max(vector.start.x, vector.end.x)) {
                if (position.y > vector.start.y) {
                    down = true;
                } else {
                    up = true;
                }
            }
        }

        if ((!left or !right) and vector.start.x == vector.end.x) {
            if (position.y > std::min(vector.start.y, vector.end.y) and position.y < std::max(vector.start.y, vector.end.y)) {
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

/*
Returns a random vec2 inside a well-formed room's exact dimensions. 
*/
template <typename T>
vec2 SpaceBuilder<T>::rejection_sample() {
    SpaceBoundingBox& box = registry.bounding_boxes.get(entity);
    std::random_device rd; 
    std::mt19937 generate(rd()); 
    // We generate random coordinates inside the bounding box. Theoretically, this could be slow if the bounding box's area is far larger than a room;
    // think of a case like an 'L'-shaped room. Since randomized levels are only generated once, and we can always code level generation
    // so that our rooms are 'decently' rectangular however, it should suffice.
    std::uniform_int_distribution<> random_x_generator(box.minimum_x, box.maximum_x); 
    std::uniform_int_distribution<> random_y_generator(box.minimum_y, box.maximum_y);
    while (true) {
        vec2 position = {random_x_generator(generate), random_y_generator(generate)};
        if (is_in_room(position)) {
            return position;
        }
    };
};

template <typename T>
vec2 SpaceBuilder<T>::get_random_position() {
    return rejection_sample();
}
