#pragma once

#include "common.hpp"
#include "render_system.hpp"
#include "tiny_ecs.hpp"
#include "oxygen.hpp"
#include "enemy.hpp"

#define LOW_OXYGEN_THRESHOLD 0.2f

// deplete oxygen (passive consumption over time)
void depleteOxygen(Entity &entity);

// affector modifies entity's oxygen/health
void modifyOxygen(Entity &entity, Entity &affector);

// helper
bool isDeadAfterChange(Oxygen &oxygen, float amount);

// checks player oxygen and renders oxygen bar (calls checkOxygenLevel)
void checkAndRenderOxygen(Entity &entity, Oxygen &oxygen, float amount);

// checks if player oxygen is low and adds/removes lowOxygen component
void checkOxygenLevel(Entity &entity);

// updates position of enemy and associated health bar
void updateHealthBarAndEnemyPos(Entity &enemy);
