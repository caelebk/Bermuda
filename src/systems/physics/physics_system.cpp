// internal
#include "physics_system.hpp"
#include "physics.hpp"
#include "player_factories.hpp"
#include "audio_system.hpp"

// Returns the local bounding coordinates scaled by the current size of the
// entity
vec2 get_bounding_box(const Position &position) {
  // abs is to avoid negative scale due to the facing direction.
  return {abs(position.scale.x), abs(position.scale.y)};
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding
// boxes and sees if the center point of either object is inside the other's
// bounding-box-circle. You can surely implement a more accurate detection
bool collides(const Position &position1, const Position &position2) {
  vec2 dp = position1.position - position2.position;
  float dist_squared = dot(dp, dp);
  const vec2 other_bonding_box = get_bounding_box(position1) / 2.f;
  const float other_r_squared = dot(other_bonding_box, other_bonding_box);
  const vec2 my_bonding_box = get_bounding_box(position2) / 2.f;
  const float my_r_squared = dot(my_bonding_box, my_bonding_box);
  const float r_squared = max(other_r_squared, my_r_squared);
  if (dist_squared < r_squared)
    return true;
  return false;
}

void PhysicsSystem::step(float elapsed_ms) {
  // Move fish based on how much time has passed, this is to (partially) avoid
  // having entities move at different speed based on the machine.
  auto &motion_registry = registry.motions;
  auto &position_registry = registry.positions;
  for (uint i = 0; i < motion_registry.size(); i++) {
    Entity entity = motion_registry.entities[i];
    Motion &motion = motion_registry.get(entity);
    Position &position = position_registry.get(entity);

    float step_seconds = elapsed_ms / 1000.f;
    vec2 distance = (motion.velocity) * step_seconds;
    position.position += distance;
  }

  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // TODO A2: HANDLE EGG UPDATES HERE
  // DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  // Check for collisions between all moving entities
  ComponentContainer<Position> &position_container = registry.positions;
  for (uint i = 0; i < position_container.components.size(); i++) {
    Position &position_i = position_container.components[i];
    Entity entity_i = position_container.entities[i];

    // note starting j at i+1 to compare all (i,j) pairs only once (and to not
    // compare with itself)
    for (uint j = i + 1; j < position_container.components.size(); j++) {
      Position &position_j = position_container.components[j];
      if (collides(position_i, position_j)) {
        Entity entity_j = position_container.entities[j];
        // Create a collisions event
        // We are abusing the ECS system a bit in that we potentially insert
        // muliple collisions for the same entity
        registry.collisions.emplace_with_duplicates(entity_i, entity_j);
        registry.collisions.emplace_with_duplicates(entity_j, entity_i);
      }
    }
  }

  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // TODO A2: HANDLE EGG collisions HERE
  // DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

void updateWepProjPos(vec2 mouse_pos, Entity player, Entity player_weapon, Entity player_projectile) {
    vec2 player_pos = registry.positions.get(player).position;
    vec2 pos_cursor_vec = mouse_pos - player_pos;
    float angle = atan2(pos_cursor_vec.y, pos_cursor_vec.x);
    Position& weapon_pos = registry.positions.get(player_weapon);
    Position& proj_pos = registry.positions.get(player_projectile);
    weapon_pos.angle = angle;
    weapon_pos.position = calculate_pos_vec(GUN_RELATIVE_POS_FROM_PLAYER.x, player_pos, weapon_pos.angle);
    if (registry.playerProjectiles.get(player_projectile).is_loaded) {
        proj_pos.angle = angle;
        proj_pos.position = calculate_pos_vec(HARPOON_RELATIVE_POS_FROM_GUN.x, weapon_pos.position, proj_pos.angle, { 0.f, HARPOON_RELATIVE_POS_FROM_GUN.y });
    }
}

void setFiredProjVelo(Entity player_projectile) {
    PlayerProjectile& proj = registry.playerProjectiles.get(player_projectile);
    proj.is_loaded = false;
    float angle = registry.positions.get(player_projectile).angle;
    registry.motions.get(player_projectile).velocity = { HARPOON_SPEED * cos(angle), HARPOON_SPEED * sin(angle) };
    registry.sounds.insert(player_projectile, Sound(blast_sound));
}
