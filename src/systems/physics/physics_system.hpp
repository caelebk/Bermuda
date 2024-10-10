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
#define HARPOON_SPEED 5.0f

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem
{
public:
	void step(float elapsed_ms);

	PhysicsSystem()
	{
	}
};

void updateWepProjPos(vec2 mouse_pos, Entity player, Entity player_weapon, Entity player_projectile);

void setFiredProjVelo(Entity player_projectile);
