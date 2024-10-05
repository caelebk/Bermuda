#pragma once

#include <functional>
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
    std::function<Position(int)> direction;
    Position pointer;

    virtual void new_entity(T entity);
    virtual void update_bounding_box(Vector& vector);
    virtual SpaceBuilder& add_wall(int magnitude);

    virtual Position get_updated_up_position(int magnitude);
    virtual Position get_updated_down_position(int magnitude);
    virtual Position get_updated_right_position(int magnitude);
    virtual Position get_updated_left_position(int magnitude);

public:
    T entity;
    SpaceBuilder();

    virtual SpaceBuilder& up(int magnitude = 0);
    virtual SpaceBuilder& down(int magnitude = 0);
    virtual SpaceBuilder& left(int magnitude = 0);
    virtual SpaceBuilder& right(int magnitude = 0);
    virtual SpaceBuilder& door(int magnitude = 0);
    virtual SpaceBuilder& connect(Entity& connector);

    // virtual SpaceBuilder* rotate_right(int rotations);
    // virtual SpaceBuilder* rotate_left(int rotations);
};

template <typename T>
SpaceBuilder<T>::SpaceBuilder() : direction(), pointer(Position(0, 0)) {}

template <typename T>
void SpaceBuilder<T>::new_entity(T entity) {
    this->entity = entity;
    registry.spaces.insert(entity, Space());
    registry.adjacencies.insert(entity, Adjacency());
    registry.bounding_boxes.insert(entity, BoundingBox());
}

template <typename T>
void SpaceBuilder<T>::update_bounding_box(Vector& vector) {
    BoundingBox& bounding_box = registry.bounding_boxes.get(entity);
    bounding_box.minimum_x = std::min(bounding_box.minimum_x, vector.end.x);
    bounding_box.maximum_x = std::max(bounding_box.maximum_x, vector.end.x);   
    bounding_box.minimum_y = std::min(bounding_box.minimum_y, vector.end.y);
    bounding_box.maximum_y = std::max(bounding_box.maximum_y, vector.end.y);  
};

template <typename T>
SpaceBuilder<T>& SpaceBuilder<T>::add_wall(int magnitude) {
    if (magnitude != 0) {
        Position endpoint = direction(magnitude);

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
Position SpaceBuilder<T>::get_updated_up_position(int magnitude) {
    return Position(pointer.x, pointer.y + magnitude);
}

template <typename T>
Position SpaceBuilder<T>::get_updated_down_position(int magnitude) {
    return Position(pointer.x, pointer.y - magnitude);
}

template <typename T>
Position SpaceBuilder<T>::get_updated_right_position(int magnitude) {
    return Position(pointer.x + magnitude, pointer.y);
}

template <typename T>
Position SpaceBuilder<T>::get_updated_left_position(int magnitude) {
    return Position(pointer.x - magnitude, pointer.y);
}

template <typename T>
SpaceBuilder<T>& SpaceBuilder<T>::up(int magnitude) {
    direction = [this](int magnitude) -> Position { return this->get_updated_up_position(magnitude); };
    return add_wall(magnitude);
}

template <typename T>
SpaceBuilder<T>& SpaceBuilder<T>::down(int magnitude) {
    direction = [this](int magnitude) -> Position { return this->get_updated_down_position(magnitude); };
    return add_wall(magnitude);
}

template <typename T>
SpaceBuilder<T>& SpaceBuilder<T>::left(int magnitude) {
    direction = [this](int magnitude) -> Position { return this->get_updated_left_position(magnitude); };
    return add_wall(magnitude);
}

template <typename T>
SpaceBuilder<T>& SpaceBuilder<T>::right(int magnitude) {
    direction = [this](int magnitude) -> Position { return this->get_updated_right_position(magnitude); };
    return add_wall(magnitude);
}

template <typename T>
SpaceBuilder<T>& SpaceBuilder<T>::door(int magnitude) {
    Position endpoint = direction(magnitude);

    Entity door = Door();
    Vector vector = Vector(pointer, endpoint);
    registry.vectors.insert(door, vector);

    Space& space = registry.spaces.get(entity);
    space.doors.push_back(door);
    space.boundaries.push_back(door);
    update_bounding_box(vector);
    pointer = endpoint;
    return *this;
}

template <typename T>
SpaceBuilder<T>& SpaceBuilder<T>::connect(Entity& connector) {
    registry.adjacencies.get(entity).neighbours.push_back(connector);
    return *this;
};