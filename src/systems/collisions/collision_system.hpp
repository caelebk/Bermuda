#pragma once

#include "abilities.hpp"
#include "ai.hpp"
#include "ai_system.hpp"
#include "audio_system.hpp"
#include "boss_factories.hpp"
#include "collision_util.hpp"
#include "common.hpp"
#include "components.hpp"
#include "debuff.hpp"
#include "enemy.hpp"
#include "environment.hpp"
#include "items.hpp"
#include "level_system.hpp"
#include "misc.hpp"
#include "oxygen.hpp"
#include "oxygen_system.hpp"
#include "physics.hpp"
#include "player.hpp"
#include "tiny_ecs.hpp"
#include "tiny_ecs_registry.hpp"

class CollisionSystem {
  private:
  RenderSystem* renderer;
  LevelSystem*  level;

  /********************
  COLLISION END DETECTION
  *********************/
  void handle_collision_end();

  /********************
  COLLISION DETECTION
  *********************/
  void collision_detection();

  void detectPlayerProjectileCollisions();
  void detectPlayerCollisions();
  void detectWallCollisions();
  void detectMassCollisions();
  void detectDoorCollisions();
  void detectEnemySupportCollisions();

  bool checkBoxCollision(Entity entity_i, Entity entity_j);
  bool checkCircleCollision(Entity entity_i, Entity entity_j);
  bool checkCircleBoxCollision(Entity circle_bound_entity,
                               Entity box_bound_entity);
  bool checkPlayerMeshCollision(Entity entity_i, Entity entity_j,
                                Entity collisionMesh);

  /********************
  COLLISION RESOLUTION
  *********************/

  void collision_resolution();
  void collision_resolution_debug_info(Entity entity, Entity entity_other);

  /***********************************************************************
  Entity -> Other Collision Routing (routes to correct Entity <-> Entity)
  ***********************************************************************/
  void routePlayerCollisions(Entity player, Entity other);
  void routeEnemyCollisions(Entity enemy, Entity other);
  void routeWallCollisions(Entity wall, Entity other);
  void routeDoorCollisions(Entity door, Entity other);
  void routePlayerProjCollisions(Entity player_proj, Entity other);
  void routeItemCollisions(Entity item, Entity other);
  void routeConsumableCollisions(Entity consumable, Entity other);
  void routeInteractableCollisions(Entity interactable, Entity other);

  /***********************************************************************
      Entity <-> Entity Collision Resolutions
  ***********************************************************************/
  // Player <-> Enemy
  void resolvePlayerEnemyCollision(Entity player, Entity enemy);

  // Player <-> Item
  void resolvePlayerItemCollision(Entity player, Entity item);

  // Player <-> Enemy Projectile
  void resolvePlayerEnemyProjCollision(Entity player, Entity enemy_proj);

  // Player <-> Consumable
  void resolvePlayerConsumableCollision(Entity player, Entity consumable);

  // Player <-> Interactable
  void resolvePlayerInteractableCollision(Entity player, Entity interactable);

  // Enemy <-> Player Projectile
  void resolveEnemyPlayerProjCollision(Entity enemy, Entity player_proj);

  // Enemy <-> Enemy Support
  void resolveEnemyEnemySupportCollision(Entity enemy, Entity enemy_support);

  // Breakable <-> Player Projectile
  void resolveBreakablePlayerProjCollision(Entity breakable,
                                           Entity player_proj);

  // Breakable <-> Enemy Projectile
  void resolveBreakableEnemyProjCollision(Entity crate, Entity enemy_proj);

  // Wall <-> Player Projectile
  void resolveWallPlayerProjCollision(Entity wall, Entity player_proj);

  // Canister <-> Player Projectile
  void resolveCanisterPlayerProjCollision(Entity canister, Entity player_proj);

  // Wall <-> Enemy Projectile
  void resolveWallEnemyProjCollision(Entity wall, Entity enemy_proj);

  // Wall <-> Enemy Projectile
  void resolveWallEnemyCollision(Entity wall, Entity enemy);

  // Wall <-> Something that should stop on the wall
  void resolveStopOnWall(Entity wall, Entity entity);

  // Crate <-> Player (Or anything with mass)
  void resolveMassCollision(Entity crate, Entity other);

  // Door <-> Player
  void resolveDoorPlayerCollision(Entity door, Entity player);

  // Mass <-> Interactable
  void resolveMassInteractableCollision(Entity mass, Entity interactable);

  /***********************************************************************
      Special Detection & Resolutions
  ***********************************************************************/

  void detectAndResolveExplosion(Entity proj, Entity enemy);

  // Note: The actual angle window used is really 2 times the angle
  // (position.angle += angle)
  void detectAndResolveConeAOE(Entity proj, Entity enemy, float angle);

  public:
  void init(RenderSystem* renderer, LevelSystem* level);
  void step(float elapsed_ms);

  CollisionSystem() {}
};

vec2 get_bounding_box(const Position& position);
vec4 get_bounds(const Position& position);
bool circle_collides(const Position& position1, const Position& position2);
bool box_collides(const Position& position1, const Position& position2);
bool circle_box_collides(const Position& position1, float radius,
                         const Position& position2);
bool mesh_collides(Entity mesh, Entity other);

extern Entity player_projectile;
extern Entity player;

extern bool   room_transitioning;
extern Entity rt_entity;
