#include "components.hpp"
#include "physics.hpp"
#include "tiny_ecs_registry.hpp"

vec2 get_bounding_box(const Position& position);
vec4 get_bounds(const Position& position);
bool circle_collides(const Position& position1, const Position& position2);
bool box_collides(const Position& position1, const Position& position2);
bool circle_box_collides(const Position& position1, float radius,
                         const Position& position2);
bool mesh_collides(Entity mesh, Entity other);
vec2 find_closest_point(const Position& pos1, const Position& pos2);
