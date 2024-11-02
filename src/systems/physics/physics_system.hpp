#pragma once

#include "abilities.hpp"
#include "ai.hpp"
#include "audio_system.hpp"
#include "common.hpp"
#include "components.hpp"
#include "enemy.hpp"
#include "environment.hpp"
#include "items.hpp"
#include "misc.hpp"
#include "oxygen.hpp"
#include "physics.hpp"
#include "player.hpp"
#include "player_factories.hpp"
#include "tiny_ecs.hpp"
#include "tiny_ecs_registry.hpp"

// Net Projectile Speed (To be balanced later)
#define HARPOON_SPEED 100.0f

// Max velocity (To be balanced later)
#define MAX_PLAYER_SPEED 30.f
#define MAX_DASH_SPEED MAX_PLAYER_SPEED * 2

// NOTE: WATER_FRICTION SHOULD ALWAYS BE SMALLER THAN PLAYER_ACCELERATION

// Velocity given by pressing movement keys (To be balanced later)
#define PLAYER_ACCELERATION MAX_PLAYER_SPEED
#define DASH_ACCELERATION PLAYER_ACCELERATION * 2

// Acceleration applies on player by force of friction
#define WATER_FRICTION MAX_PLAYER_SPEED / 2.f

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem {
  private:
  public:
  void step(float elapsed_ms);

  PhysicsSystem() {}
};

extern bool   paused;
extern Entity player;

void updateWepProjPos(vec2 mouse_pos);

void setFiredProjVelo();

void setPlayerAcceleration();

void calculatePlayerVelocity(float lerp);

void applyWaterFriction(Entity entity);