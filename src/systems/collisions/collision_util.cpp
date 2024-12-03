#include "collision_util.hpp"
#include <player_factories.hpp>
#include <consumable_factories.hpp>

// Returns the local bounding coordinates scaled by entity size
vec2 get_bounding_box(const Position& position) {
  return {abs(position.scale.x), abs(position.scale.y)};
}

// Returns the rectangular bounds
vec4 get_bounds(const Position& position) {
  vec2  bounding_box        = get_bounding_box(position);
  float horizontal_dist_box = (bounding_box.x) / 2.0f;
  float vertical_dist_box   = (bounding_box.y) / 2.0f;

  float left_bound  = position.position.x - horizontal_dist_box;
  float right_bound = position.position.x + horizontal_dist_box;
  float top_bound   = position.position.y - vertical_dist_box;
  float bot_bound   = position.position.y + vertical_dist_box;

  return vec4(left_bound, right_bound, top_bound, bot_bound);
}

bool circle_collides(const Position& position1, const Position& position2) {
  vec2        dp                = position1.position - position2.position;
  float       dist_squared      = dot(dp, dp);
  const vec2  other_bonding_box = get_bounding_box(position1) / 2.f;
  const float other_r_squared   = dot(other_bonding_box, other_bonding_box);
  const vec2  my_bonding_box    = get_bounding_box(position2) / 2.f;
  const float my_r_squared      = dot(my_bonding_box, my_bonding_box);
  const float r_squared         = max(other_r_squared, my_r_squared);
  return dist_squared < r_squared;
}

// Axis-Aligned Bounding Box Collision detection.
bool box_collides(const Position& position1, const Position& position2) {
  vec4 box1_bounds = get_bounds(position1);

  float left_bound_box1  = box1_bounds[0];
  float right_bound_box1 = box1_bounds[1];
  float top_bound_box1   = box1_bounds[2];
  float bot_bound_box1   = box1_bounds[3];

  vec4 box2_bounds = get_bounds(position2);

  float left_bound_box2  = box2_bounds[0];
  float right_bound_box2 = box2_bounds[1];
  float top_bound_box2   = box2_bounds[2];
  float bot_bound_box2   = box2_bounds[3];

  bool vertical_overlap    = top_bound_box1 < bot_bound_box2;
  bool vertical_overlap2   = top_bound_box2 < bot_bound_box1;
  bool horizontal_overlap  = left_bound_box1 < right_bound_box2;
  bool horizontal_overlap2 = left_bound_box2 < right_bound_box1;

  return vertical_overlap && vertical_overlap2 && horizontal_overlap &&
         horizontal_overlap2;
}

bool circle_box_collides(const Position& circle_pos, float radius,
                         const Position& box_pos) {
  vec4 box_bound = get_bounds(box_pos);

  float box_left  = box_bound[0];
  float box_right = box_bound[1];
  float box_top   = box_bound[2];
  float box_bot   = box_bound[3];

  float closestX = clamp(circle_pos.position.x, box_left, box_right);
  float closestY = clamp(circle_pos.position.y, box_top, box_bot);

  float distanceX = circle_pos.position.x - closestX;
  float distanceY = circle_pos.position.y - closestY;

  float distanceSquared = distanceX * distanceX + distanceY * distanceY;
  float radiusSquared   = radius * radius;

  return distanceSquared <= radiusSquared;
}

bool mesh_collides(Entity mesh, Entity other) {
  // ignore player collision mesh - player collisions
  if (registry.players.has(other) || !registry.positions.has(mesh) ||
      !registry.meshPtrs.has(mesh)) {
    return false;
  }

  Position& mesh_pos  = registry.positions.get(mesh);
  Mesh*     meshPtr   = registry.meshPtrs.get(mesh);
  Position& other_pos = registry.positions.get(other);

  vec4 other_bb = get_bounds(other_pos);

  // shockwave and canister explosions use circle mesh collision
  bool is_shockwave =
      registry.enemyProjectiles.has(other) &&
      registry.enemyProjectiles.get(other).type == ENTITY_TYPE::SHOCKWAVE;
  bool is_canister =
      (registry.enemyProjectiles.has(other) &&
       registry.enemyProjectiles.get(other).type ==
           ENTITY_TYPE::OXYGEN_CANISTER) ||
      (registry.consumables.has(other) &&
       registry.consumables.get(other).type == ENTITY_TYPE::OXYGEN_CANISTER);
  float radius = 0.f;
  if (is_shockwave) {
    radius = max(other_pos.scale.x, other_pos.scale.y) / 2;
  } else if (is_canister && registry.aoe.has(other)) {
    radius = registry.aoe.get(other).radius;
  }
  bool is_circle = is_shockwave || is_canister;

  // get transformations
  Transform transform;
  transform.translate(mesh_pos.position);
  transform.rotate(mesh_pos.angle);
  transform.scale(mesh_pos.scale);
  mat3 modelmatrix = transform.mat;

  for (uint16_t i = 0; i < meshPtr->vertex_indices.size(); i += 3) {
    bool hor  = false;
    bool vert = false;
    if ((size_t)i + 2 >= meshPtr->vertices.size()) {
      continue;
    }

    std::vector<vec2> overlap;
    std::vector<vec2> non_overlap;
    for (uint16_t idx = 0; idx < 3; idx++) {
      // get vertex
      ColoredVertex v = meshPtr->vertices[i + idx];
      // convert to world coordinates
      v.position.z = 1.f;
      v.position   = modelmatrix * v.position;

      if (is_circle) {
        vec2  v_point = v.position;
        float dist_squared =
            dot(v_point - other_pos.position, v_point - other_pos.position);
        if (dist_squared <= radius * radius) {
          return true;
        } else {
          non_overlap.push_back(v.position);
        }
      } else {
        bool hor_local =
            (v.position.x >= other_bb[0] && v.position.x <= other_bb[1]);
        bool ver_local =
            (v.position.y >= other_bb[2] && v.position.y <= other_bb[3]);

        hor  = hor || hor_local;
        vert = vert || ver_local;

        if (hor_local || ver_local) {
          overlap.push_back(v.position);
        }
      }
    }
    // is colliding with 1 vertex
    if (overlap.size() == 0 || !hor || !vert) {
      if (is_circle && non_overlap.size() == 3) {
        // no vertex is in circle, check if midpoints are in circle
        std::vector<vec2> midpoints = {
            (non_overlap[0] + non_overlap[1]) / 2.f,
            (non_overlap[1] + non_overlap[2]) / 2.f,
            (non_overlap[0] + non_overlap[2]) / 2.f};
        for (vec2& midpoint : midpoints) {
          float dist_squared =
              dot(midpoint - other_pos.position, midpoint - other_pos.position);
          if (dist_squared <= radius * radius) {
            return true;
          }
        }
      }
      continue;
    }

    if (overlap.size() != 2) {
      // this means that a single vertex is inside, or all 3 (the entire thing)
      // is inside
      return true;
    }

    vec2 vert0 = overlap[0];
    vec2 vert1 = overlap[1];
    // the midpoint should also be overlapping
    vec2 midpoint = (vert0 + vert1) / 2.f;

    if ((midpoint.x >= other_bb[0] && midpoint.x <= other_bb[1]) &&
        (midpoint.y >= other_bb[2] && midpoint.y <= other_bb[3])) {
      return true;
    }
  }

  return false;
}

vec2 find_closest_point(const Position& pos1, const Position& pos2) {
  vec4 wall_box  = get_bounds(pos2);
  vec2 closest_p = {0.f, 0.f};
  // find closest x point
  if (wall_box[0] < pos1.position.x && wall_box[1] > pos1.position.x) {
    closest_p.x = pos1.position.x;
  } else if (abs(wall_box[0] - pos1.position.x) <
             abs(wall_box[1] - pos1.position.x)) {
    closest_p.x = wall_box[0];
  } else {
    closest_p.x = wall_box[1];
  }

  // find closest y point
  if (wall_box[2] < pos1.position.y && wall_box[3] > pos1.position.y) {
    closest_p.y = pos1.position.y;
  } else if (abs(wall_box[2] - pos1.position.y) <
             abs(wall_box[3] - pos1.position.y)) {
    closest_p.y = wall_box[2];
  } else {
    closest_p.y = wall_box[3];
  }

  return closest_p;
}

Entity make_canister_explosion(RenderSystem* renderer, vec2 pos) {
  return makeExplosion(
      renderer, pos, EXPLOSION_DURATION,
      CANISTER_EXPLOSION_BOUNDING_BOX * CANISTER_EXPLOSION_SCALE_FACTOR);
}