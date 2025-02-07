#pragma once

#include "common.hpp"
#include "enemy.hpp"
#include "oxygen.hpp"
#include "render_system.hpp"
#include "tiny_ecs.hpp"

#define LOW_OXYGEN_THRESHOLD 0.2f

// deplete oxygen (passive consumption over time)
void depleteOxygen(Entity& entity);

// affector modifies entity's oxygen/health
void modifyOxygen(Entity& entity, Entity& affector);

void modifyOxygenAmount(Entity& entity, float amount);

// update position of enemy health bars
void updateEnemyHealthBarPos(Entity& enemy);

// wrapper
float oxygen_drain(float oxygen_deplete_timer,
                   float elapsed_ms_since_last_update);

// helper
bool isModOnCooldown(Entity& oxygenModifier);

// helper
float calcDeltaOxygen(Oxygen& entity_oxygen, float oxygenModifierAmount);

// helper
void updateHealthBarRender(Entity& entity, Oxygen& entity_oxygen,
                           float deltaOxygen);

// helper
void updateOxygenLvlStatus(Oxygen& entity_oxygen);

// helper
void updateDeathStatus(Entity& entity, Oxygen& entity_oxygen);

void createDefaultHealthbar(RenderSystem* renderer, Entity& entity,
                            float health, vec2 healthScale, vec2 barScale,
                            vec2 bounding_box);

void renderHealthBar(Oxygen& entity_oxygen);
