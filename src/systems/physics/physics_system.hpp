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
#define SHRIMP_SPEED 500.0f

// Max velocity (To be balanced later)
#define MAX_PLAYER_SPEED 40.f
#define MAX_GLIDE_SPEED MAX_PLAYER_SPEED * 3

// Player Dash
#define DASH_DURATION 200.f
#define DASH_COOLDOWN_DURATION 500.f
#define DASH_SPEED 300
// NOTE: WATER_FRICTION SHOULD ALWAYS BE SMALLER THAN PLAYER_ACCELERATION

// Velocity given by pressing movement keys (To be balanced later)
#define PLAYER_ACCELERATION MAX_PLAYER_SPEED
#define GLIDE_ACCELERATION PLAYER_ACCELERATION * 2

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

void updatePlayerDirection(vec2 mouse_pos);

void setFiredProjVelo();

void setPlayerAcceleration();

void calculatePlayerVelocity(float lerp);

void calculateVelocity(Entity entity, float lerp);


void playerDash(float elapsed_ms);

void applyWaterFriction(Entity entity);