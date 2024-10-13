#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "audio_system.hpp"
#include "oxygen_system.hpp"

#include "abilities.hpp"
#include "debuff.hpp"
#include "ai.hpp"
#include "enemy.hpp"
#include "environment.hpp"
#include "items.hpp"
#include "misc.hpp"
#include "oxygen.hpp"
#include "physics.hpp"
#include "player.hpp"

class CollisionSystem
{

private:
	void collision_detection();
	void collision_resolution();
	void collision_resolution_debug_info(Entity entity, Entity entity_other);

    /***********************************************************************
    Entity -> Other Collision Routing (routes to correct Entity <-> Entity)
    ***********************************************************************/
	void routePlayerCollisions(Entity player, Entity other);
    void routeEnemyCollisions(Entity enemy, Entity other);
	void routeWallCollisions(Entity wall, Entity other);
	void routePlayerProjCollisions(Entity player_proj, Entity other);
    void routeConsumableCollisions(Entity consumable, Entity other);
    void routeInteractableCollisions(Entity interactable, Entity other);

    /***********************************************************************
        Entity <-> Entity Collision Resolutions
    ***********************************************************************/    
    //Player <-> Enemy
    void resolvePlayerEnemyCollision(Entity player, Entity enemy);

    //Player <-> Consumable
    void resolvePlayerConsumableCollision(Entity player, Entity consumable);

    //Player <-> Interactable
    void resolvePlayerInteractableCollision(Entity player, Entity interactable);

    //Enemy <-> Player Projectile
    void resolveEnemyPlayerProjCollision(Entity enemy, Entity player_proj);

    //Wall <-> Player Projectile
    void resolveWallPlayerProjCollision(Entity wall, Entity player_proj);

    //Wall <-> Player Projectile
    void resolveWallEnemyCollision(Entity wall, Entity enemy);

    //Wall <-> Player
    void resolveWallPlayerCollision(Entity wall, Entity player);

public:
	void step(float elapsed_ms);

	CollisionSystem()
	{
	}
};

vec2 get_bounding_box(const Position &position);
vec4 get_bounds(const Position &position);
bool circle_collides(const Position &position1, const Position &position2);
bool box_collides(const Position &position1, const Position &position2);