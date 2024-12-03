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

// Net Projectile Speed
#define HARPOON_SPEED 200.0f
#define SHRIMP_SPEED 500.0f

// Max velocity
#define MAX_PLAYER_SPEED 50.f
#define MAX_GLIDE_SPEED MAX_PLAYER_SPEED * 3

// Player Dash
#define DASH_DURATION 200.f
#define DASH_COOLDOWN_DURATION 500.f
#define DASH_SPEED 300
// NOTE: WATER_FRICTION SHOULD ALWAYS BE SMALLER THAN PLAYER_ACCELERATION

// Velocity given by pressing movement keys
#define PLAYER_ACCELERATION MAX_PLAYER_SPEED
#define GLIDE_ACCELERATION PLAYER_ACCELERATION * 2

// Acceleration applies on player by force of friction
#define WATER_FRICTION MAX_PLAYER_SPEED / 2.f

// Geyser Bubbles
#define BUBBLE_SCALE_FACTOR vec2(1.f)
#define BUBBLE_BOUNDING_BOX vec2(14.f, 14.f)
#define BUBBLE_INTERVAL 500.f
#define INITIAL_BUBBLE_VELOCITY {0.f, -30.f} 

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem {
  private:
  public:
  void step(float elapsed_ms);

  PhysicsSystem() {}
};

extern bool   is_paused;
extern Entity player;

Entity createGeyserBubble(RenderSystem* renderer, vec2 pos);

void updateWepProjPos(vec2 mouse_pos);

void updatePlayerDirection(vec2 mouse_pos);

void setFiredProjVelo();

void setPlayerAcceleration();

void calculatePlayerVelocity(float lerp);

void calculateVelocity(Entity entity, float lerp);


void playerDash(float elapsed_ms);

void applyWaterFriction(Entity entity);