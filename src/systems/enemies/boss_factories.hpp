#pragma once

#include "common.hpp"
#include "random.hpp"
#include "render_system.hpp"
#include "tiny_ecs.hpp"
#include "collision_system.hpp"
#include "components.hpp"
#include "oxygen_system.hpp"
#include "physics.hpp"
#include "random.hpp"
#include "tiny_ecs_registry.hpp"

//////////////////////////////////////////////////////////////
// Crab Boss
//////////////////////////////////////////////////////////////
#define KRAB_BOSS_MS 80.0
#define KRAB_BOSS_DAMAGE -300.0
#define KRAB_BOSS_ATK_SPD 1000.0
#define KRAB_BOSS_SCALE vec2(0.1)
#define KRAB_BOSS_BOUNDING_BOX vec2(3433.f, 2494.f)
#define KRAB_BOSS_HEALTH 500.0
#define KRAB_BOSS_MIN_DIR_CD 1000 // random direction change cooldown so it looks more natural
#define KRAB_BOSS_MAX_DIR_CD 8000
#define KRAB_BOSS_TRACKING_CD 1000.f
#define KRAB_BOSS_ENEMY_LIMIT 16 // 15 + itself
#define KRAB_BOSS_SPOT_RADIUS 400.f
#define KRAB_BOSS_LEASH_RADIUS 600.f
#define KRAB_BOSS_TRACKING_ACCELERATION 10.f
#define KRAB_BOSS_HEALTH_SCALE vec2(1.4f)
#define KRAB_BOSS_HEALTH_BAR_SCALE vec2(1.5f)
#define KRAB_BOSS_HEALTH_BOUNDING_BOX vec2(50.f, 5.f)
#define KRAB_BOSS_AI_CD 5000 // switch every 10 seconds

// Create tutorial jellyfish.
Entity createJellyBossPos(RenderSystem* renderer, vec2 position, bool checkCollisions);
Entity createCrabBossPos(RenderSystem* renderer, vec2 position, bool checkCollisions = true);
Entity respawnCrabBoss(RenderSystem* renderer, EntityState es);

