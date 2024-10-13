#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

#include "abilities.hpp"
#include "ai.hpp"
#include "enemy.hpp"
#include "environment.hpp"
#include "items.hpp"
#include "misc.hpp"
#include "oxygen.hpp"
#include "physics.hpp"
#include "player.hpp"

// Net Projectile Speed (To be balanced later)
#define HARPOON_SPEED 100.0f

// Max velocity (To be balanced later)
#define MAX_PLAYER_SPEED 20.f
#define MAX_DASH_SPEED MAX_PLAYER_SPEED*2

// NOTE: WATER_FRICTION SHOULD ALWAYS BE SMALLER THAN PLAYER_ACCELERATION

// Velocity given by pressing movement keys (To be balanced later)
#define PLAYER_ACCELERATION 20.f
#define DASH_ACCELERATION PLAYER_ACCELERATION*2

// Acceleration applies on player by force of friction
#define WATER_FRICTION 10.f


// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem
{

private:
	void collision_detection();
	void collision_resolution();
	void collision_resolution_debug_info(Entity entity, Entity entity_other);
	void resolvePlayerCollisions(Entity player, Entity other);
	void resolveWallCollisions(Entity wall, Entity other);
	void resolvePlayerProjCollisions(Entity player_proj, Entity other);
public:
	void step(float elapsed_ms);

	PhysicsSystem()
	{
	}
};

void updateWepProjPos(vec2 mouse_pos, Entity player, Entity player_weapon, Entity player_projectile);

void setFiredProjVelo(Entity player_projectile);

void setPlayerAcceleration(Entity player);

void calculatePlayerVelocity(Entity player, float lerp);

void applyWaterFriction(Entity entity);
